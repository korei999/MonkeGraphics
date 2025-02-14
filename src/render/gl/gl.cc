#include "gl.hh"

#include "app.hh"
#include "asset.hh"
#include "common.hh"
#include "control.hh"
#include "game/game.hh"
#include "shaders/glsl.hh"

#include "adt/defer.hh"
#include "adt/Map.hh"
#include "adt/OsAllocator.hh"
#include "adt/logs.hh"
#include "adt/ScratchBuffer.hh"
#include "adt/file.hh"

using namespace adt;

namespace render::gl
{

/* used for gltf::Primitive::extras::pData */
struct PrimitiveData
{
    GLuint vao {};
    GLuint vbo {};
    GLuint ebo {};
};

static void loadShaders();
static void loadAssetObjects();

Pool<Shader, 128> g_aShaders(adt::INIT);
static Map<String, PoolHnd> s_mapStringToShaders(OsAllocatorGet(), g_aShaders.getCap());

static u8 s_aScratchMem[SIZE_8K] {};
static ScratchBuffer s_scratch(s_aScratchMem);

static const ShaderMapping s_aShadersToLoad[] {
    {shaders::glsl::ntsQuadTexVert, shaders::glsl::ntsQuadTexFrag, "QuadTex"},
    {shaders::glsl::ntsSimpleColorVert, shaders::glsl::ntsSimpleColorFrag, "SimpleColor"},
    {shaders::glsl::ntsSimpleTextureVert, shaders::glsl::ntsSimpleTextureFrag, "SimpleTexture"},
};

void
Renderer::init()
{
#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallbackARB(gl::debugCallback, app::g_pWindow);
#endif

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glCullFace(GL_FRONT);

    loadShaders();
    loadAssetObjects();
};

static void
drawGLTFNode(gltf::Model* pModel, gltf::Node* pNode, math::M4 trm)
{
    using namespace adt::math;

    common::updateModelNode(pModel, pNode, &trm);

    for (auto& children : pNode->vChildren)
    {
        auto& childNode = pModel->m_vNodes[children];
        drawGLTFNode(pModel, &childNode, trm);
    }

    auto* pModelObj = reinterpret_cast<asset::Object*>(pModel);

    if (pNode->meshI != -1)
    {
        auto& mesh = pModel->m_vMeshes[pNode->meshI];
        for (const auto& primitive : mesh.vPrimitives)
        {
            gltf::Accessor& accIndices = pModel->m_vAccessors[primitive.indicesI];
            if (primitive.indicesI != -1)
                accIndices = pModel->m_vAccessors[primitive.indicesI];

            Shader* pSh {};

            /* TODO: there might be any number of TEXCOORD_*,
             * which would be specified in baseColorTexture.texCoord.
             * But current gltf parser only reads the 0th. */
            gltf::Accessor accUV {};
            if (primitive.attributes.TEXCOORD_0 != -1)
                accUV = pModel->m_vAccessors[primitive.attributes.TEXCOORD_0];

            if (primitive.materialI != -1)
            {
                auto& mat = pModel->m_vMaterials[primitive.materialI];

                if (mat.pbrMetallicRoughness.baseColorTexture.index != -1)
                {
                    auto& tex = pModel->m_vTextures[mat.pbrMetallicRoughness.baseColorTexture.index];
                    auto& img = pModel->m_vImages[tex.sourceI];

                    Span sp = s_scratch.nextMemZero<char>(img.sUri.getSize() + 300);
                    file::replacePathEnding(&sp, pModelObj->m_sMappedWith, img.sUri);

                    auto* pObj = asset::search(sp, asset::Object::TYPE::IMAGE);
                    auto* pTex = reinterpret_cast<Texture*>(pObj->pExtraData);

                    if (pTex)
                    {
                        pSh = searchShader("SimpleTexture");
                        pSh->use();
                        pTex->bind(GL_TEXTURE0);
                    }
                    else goto defaultShader;
                }
                else
                {
                    pSh = searchShader("SimpleColor");
                    pSh->use();
                    pSh->setV4("u_color", mat.pbrMetallicRoughness.baseColorFactor);
                }
            }
            else
            {
defaultShader:
                pSh = searchShader("SimpleColor");
                pSh->use();
                pSh->setV4("u_color", V4From(colors::get(colors::IDX::PINK), 1.0f));
            }

            if (pSh)
                pSh->setM4("u_trm", trm);

            auto* pPrimitiveData = reinterpret_cast<PrimitiveData*>(primitive.extras.pData);
            if (pPrimitiveData)
                glBindVertexArray(pPrimitiveData->vao);

            if (primitive.indicesI != -1)
            {
                glDrawElements(
                    static_cast<GLenum>(primitive.eMode),
                    accIndices.count,
                    static_cast<GLenum>(accIndices.eComponentType),
                    {}
                );
            }
            else
            {
                const auto& accPos = pModel->m_vAccessors[primitive.attributes.POSITION];
                glDrawArrays(static_cast<GLenum>(primitive.eMode), 0, accPos.count);
            }
        }
    }
}

static void
drawModel(gltf::Model* pModel, math::M4 trm)
{
    auto& scene = pModel->m_vScenes[pModel->m_defaultScene.nodeI];
    for (auto& nodeI : scene.vNodes)
        drawGLTFNode(pModel, &pModel->m_vNodes[nodeI], trm);
}

void
Renderer::drawEntities([[maybe_unused]] Arena* pArena)
{
    using namespace adt::math;

    auto& win = app::window();

    const auto& camera = control::g_camera;
    const f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        for (int entityI = 0; entityI < game::g_aEntites.m_size; ++entityI)
        {
            const auto& arrays = game::g_aEntites.m_arrays;
            if (arrays.abDead[entityI] || arrays.priv.abFree[entityI])
                continue;

            game::EntityBind entity = game::g_aEntites[ game::Entity{.i = entityI} ];

            auto& obj = asset::g_aObjects[entity.assetI];
            switch (obj.m_eType)
            {
                default: break;

                case asset::Object::TYPE::MODEL:
                {
                    M4 trm = M4Pers(toRad(camera.m_fov), aspectRatio, 0.01f, 1000.0f) *
                        camera.m_trm *
                        M4TranslationFrom(entity.pos) *
                        QtRot(entity.rot) *
                        M4ScaleFrom(entity.scale);

                    drawModel(&obj.m_uData.model, trm);
                }
                break;
            }
        }
    }
}

ShaderMapping::ShaderMapping(const String svVert, const String svFrag, const String svMappedTo)
    : m_svVert(svVert), m_svFrag(svFrag), m_svMappedTo(svMappedTo), m_eType(TYPE::VS_FS) {}

Texture::Texture(int width, int height)
    : m_width(width), m_height(height)
{
    loadRGBA(nullptr);
}

Texture::Texture(const adt::Span2D<ImagePixelRGBA> spImg)
    : m_width(spImg.getWidth()), m_height(spImg.getHeight())
{
    loadRGBA(spImg.data());
}

void
Texture::subImage(const Span2D<ImagePixelRGBA> spImg)
{
    ADT_ASSERT(
        spImg.getStride() > 0 && m_width <= spImg.getStride() &&
        spImg.getHeight() > 0 && m_height <= spImg.getHeight(),
        "invalid spImg size"
    );

    bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, spImg.getStride(), spImg.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, spImg.data());
}

void
Texture::destroy()
{
}

void
Texture::loadRGBA(const ImagePixelRGBA* pData)
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Shader::Shader(const adt::String sVertexShader, const adt::String sFragmentShader, const adt::String svMapTo)
{
    GLint linked {};
    GLuint vertex = loadOne(GL_VERTEX_SHADER, sVertexShader);
    GLuint fragment = loadOne(GL_FRAGMENT_SHADER, sFragmentShader);

    m_id = glCreateProgram();
    ADT_ASSERT(m_id != 0, "glCreateProgram failed: %u", m_id);

    glAttachShader(m_id, vertex);
    glAttachShader(m_id, fragment);

    glLinkProgram(m_id);
    glGetProgramiv(m_id, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        char infoLog[255] {};
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
            glGetProgramInfoLog(m_id, infoLen, nullptr, infoLog);

        LOG_BAD("error linking program: {}\n", infoLog);
        glDeleteProgram(m_id);
        exit(1);
    }

#ifndef NDEBUG
    glValidateProgram(m_id);
#endif

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    s_mapStringToShaders.insert(svMapTo, g_aShaders.push(*this));
}

GLuint
Shader::loadOne(GLenum type, adt::String sShader)
{
    GLuint shader = glCreateShader(type);
    if (!shader)
        return 0;

    const char* srcData = sShader.data();

    glShaderSource(shader, 1, &srcData, nullptr);
    glCompileShader(shader);

    GLint ok {};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        LOG("\n{}\n", sShader);

        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            char aBuff[512] {};
            glGetShaderInfoLog(shader, infoLen, nullptr, aBuff);
            LOG_BAD("error compiling shader:\n{}\n", aBuff);
            exit(1);
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void
Shader::queryActiveUniforms()
{
    GLint maxUniformLen {};
    GLint nUniforms {};

    glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &nUniforms);
    glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLen);

    char uniformName[255] {};
    LOG_OK("queryActiveUniforms for '{}':\n", m_id);

    for (int i = 0; i < nUniforms; ++i)
    {
        GLint size {};
        GLenum type {};
        String typeName {};

        glGetActiveUniform(m_id, i, maxUniformLen, nullptr, &size, &type, uniformName);
        switch (type)
        {
            case GL_FLOAT:
            typeName = "GL_FLOAT";
            break;

            case GL_FLOAT_VEC2:
            typeName = "GL_FLOAT_VEC2";
            break;

            case GL_FLOAT_VEC3:
            typeName = "GL_FLOAT_VEC3";
            break;

            case GL_FLOAT_VEC4:
            typeName = "GL_FLOAT_VEC4";
            break;

            case GL_FLOAT_MAT4:
            typeName = "GL_FLOAT_MAT4";
            break;

            case GL_FLOAT_MAT3:
            typeName = "GL_FLOAT_MAT3";
            break;

            case GL_SAMPLER_2D:
            typeName = "GL_SAMPLER_2D";
            break;

            default:
            typeName = "unknown";
            break;
        }

        LOG_OK("\tuniformName: '{}', type: '{}'\n", uniformName, typeName);
    }
}

void
Shader::destroy()
{
    glDeleteProgram(m_id);
    LOG_NOTIFY("shader '{}' destroyed\n", m_id);
    *this = {};
}

Quad::Quad(adt::INIT_FLAG)
{
    const f32 aVertices[] = {
        /* pos(0, 1)      uv(2, 3) */
        -1.0f,  1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,    1.0f, 0.0f,

        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
         1.0f,  1.0f,    1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(aVertices), aVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    LOG_GOOD("Quad: '{}' created\n", m_vao);
}

Shader*
searchShader(const adt::String svKey)
{
    auto res = s_mapStringToShaders.search(svKey);
    if (!res)
    {
        LOG_WARN("not found\n");
        return {};
    }

    return &g_aShaders[res.value()];
}

void
loadShaders()
{
    for (const auto& shader : s_aShadersToLoad)
    {
        switch (shader.m_eType)
        {
            case ShaderMapping::TYPE::VS_FS:
            {
                /* maps to gl::g_shaders */
                gl::Shader(shader.m_svVert, shader.m_svFrag, shader.m_svMappedTo);
            }
            break;

            case ShaderMapping::TYPE::VS_GS_FS:
            ADT_ASSERT(false, "TODO");
            break;
        }
    }
}

[[maybe_unused]] static int
componentByteSize(gltf::COMPONENT_TYPE eType)
{
    switch (eType)
    {
        case gltf::COMPONENT_TYPE::UNSIGNED_BYTE:
        return static_cast<int>(sizeof(GLubyte));

        case gltf::COMPONENT_TYPE::UNSIGNED_SHORT:
        return static_cast<int>(sizeof(GLushort));

        case gltf::COMPONENT_TYPE::UNSIGNED_INT:
        return static_cast<int>(sizeof(GLuint));

        case gltf::COMPONENT_TYPE::BYTE:
        return static_cast<int>(sizeof(GLbyte));

        case gltf::COMPONENT_TYPE::SHORT:
        return static_cast<int>(sizeof(GLshort));

        case gltf::COMPONENT_TYPE::FLOAT:
        return static_cast<int>(sizeof(GLfloat));
    }

    char aBuff[127] {};
    print::toSpan(aBuff, "invalid component type: '{}'", eType);
    ADT_ASSERT(false, "%s", aBuff);
    return 0;
}

static void
loadImage(Image* pImage)
{
    if (!pImage)
    {
        LOG_WARN("pImage: {}\n", pImage);
        return;
    }

    auto& obj = *reinterpret_cast<asset::Object*>(pImage);
    LOG_GOOD("loading image '{}'...\n", obj.m_sMappedWith);

    obj.pExtraData = obj.m_arena.alloc<Texture>(pImage->getSpanRGBA());
}

static void
loadModel(gltf::Model* pModel)
{
    if (!pModel)
    {
        LOG_WARN("pModel: {}\n", pModel);
        return;
    }

    auto& obj = *reinterpret_cast<asset::Object*>(pModel);

    Vec<GLuint> vVBOs(OsAllocatorGet());;
    defer( vVBOs.destroy() );
    {
        for (auto& buffer : pModel->m_vBuffers)
        {
            GLuint vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, buffer.byteLength, buffer.sBin.data(), GL_STATIC_DRAW);
            vVBOs.push(vbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /* if no buffers there is nothing to do */
    if (vVBOs.empty())
        return;

    {
        for (auto& mesh : pModel->m_vMeshes)
        {
            LOG_GOOD("loading mesh: '{}'...\n", mesh.sName);
            for (auto& primitive : mesh.vPrimitives)
            {
                ADT_ASSERT(primitive.indicesI != -1, "TODO: deal with non non indexed arrays");

                auto& accInd = pModel->m_vAccessors[primitive.indicesI];
                auto& viewInd = pModel->m_vBufferViews[accInd.bufferViewI];
                auto& buffInd = pModel->m_vBuffers[viewInd.bufferI];

                PrimitiveData primitiveData {};

                glGenVertexArrays(1, &primitiveData.vao);
                glBindVertexArray(primitiveData.vao);

                {
                    /* NOTE: we are duplicating index buffer here.
                     * we can try glBufferStorage (4.4) */
                    glGenBuffers(1, &primitiveData.ebo);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitiveData.ebo);
                    glBufferData(
                        GL_ELEMENT_ARRAY_BUFFER,
                        viewInd.byteLength,
                        &buffInd.sBin[accInd.byteOffset + viewInd.byteOffset],
                        GL_STATIC_DRAW
                    );
                }

                {
                    ADT_ASSERT(primitive.attributes.POSITION != -1, " ");
                    auto& accPos = pModel->m_vAccessors[primitive.attributes.POSITION];
                    auto& viewPos = pModel->m_vBufferViews[accPos.bufferViewI];
                    auto& buffPos = pModel->m_vBufferViews[viewPos.bufferI];

                    primitiveData.vbo = vVBOs[buffPos.bufferI];
                    glBindBuffer(GL_ARRAY_BUFFER, primitiveData.vbo);

                    /* positions */
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(
                        0, /* enabled index */
                        3, /* positions are 3 f32s */
                        static_cast<GLenum>(accPos.eComponentType),
                        false,
                        viewPos.byteStride,
                        reinterpret_cast<void*>(accPos.byteOffset + viewPos.byteOffset)
                    );
                }

                /* uv's */
                if (primitive.attributes.TEXCOORD_0 != -1)
                {
                    gltf::Accessor accUV = pModel->m_vAccessors[primitive.attributes.TEXCOORD_0];
                    gltf::BufferView viewUV = pModel->m_vBufferViews[accUV.bufferViewI];
                    gltf::Buffer buffUV  = pModel->m_vBuffers[viewUV.bufferI];

                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(
                        1, /* enabled index */
                        2, /* uv's are 2 f32s */
                        static_cast<GLenum>(accUV.eComponentType),
                        false,
                        viewUV.byteStride,
                        reinterpret_cast<void*>(accUV.byteOffset + viewUV.byteOffset)
                    );
                }

                primitive.extras.pData = obj.m_arena.alloc<PrimitiveData>(primitiveData);
            }
        }
    }
}

void
loadAssetObjects()
{
    if (asset::g_aObjects.empty())
        LOG_WARN("asset::g_aObjects.empty(): {}\n", asset::g_aObjects.empty());

    for (auto& obj : asset::g_aObjects)
    {
        switch (obj.m_eType)
        {
            case asset::Object::TYPE::MODEL:
            loadModel(&obj.m_uData.model);
            break;

            case asset::Object::TYPE::IMAGE:
            loadImage(&obj.m_uData.img);
            break;

            case asset::Object::TYPE::NONE:
            break;
        }
    }
}

#ifndef NDEBUG

void
debugCallback(
    [[maybe_unused]] GLenum source,
    [[maybe_unused]] GLenum type,
    [[maybe_unused]] GLuint id,
    [[maybe_unused]] GLenum severity,
    [[maybe_unused]] GLsizei length,
    [[maybe_unused]] const GLchar* message,
    [[maybe_unused]] const void* user
)
{
    const char* typeStr {};
    const char* sourceStr {};

    switch (source)
    {
        case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;

        default: break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;

        default: break;
    }

    LOG_WARN("source: '{}', type: '{}'\n{}\n", sourceStr, typeStr, message);
}

#endif

} /* namespace render::gl */
