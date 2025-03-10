#include "gl.hh"

#include "app.hh"
#include "Model.hh"
#include "asset.hh"
#include "common.hh"
#include "control.hh"
#include "game/game.hh"
#include "shaders/glsl.hh"

#include "adt/ThreadPool.hh"
#include "adt/defer.hh"
#include "adt/Map.hh"
#include "adt/StdAllocator.hh"
#include "adt/View.hh"
#include "adt/logs.hh"
#include "adt/ScratchBuffer.hh"
#include "adt/file.hh"

using namespace adt;

namespace render::gl
{

/* used for gltf::Primitive::pData */
struct PrimitiveData
{
    GLuint vao {};
    GLuint vbo {};
    GLuint vboJoints {};
    GLuint vboWeights {};
    GLuint ebo {};
};

struct Skybox
{
    GLuint m_fbo;
    GLuint m_tex;
    int m_width;
    int m_height;

    /* */

    Skybox() = default;
    Skybox(Image a6Images[6]);

    /* */

    void bind() { glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex); }
};

static void loadShaders();
static void loadAssetObjects();
static void loadSkybox();

Pool<Shader, 128> g_poolShaders {};
static MapManaged<StringView, PoolHandle<Shader>> s_mapStringToShaders(StdAllocator::inst(), g_poolShaders.cap());

static u8 s_aScratchMem[SIZE_1K * 100] {};
static ScratchBuffer s_scratch(s_aScratchMem);

static Texture s_texDefault {};
static Skybox s_skyboxDefault {};

static ThreadPool s_threadPool(StdAllocator::inst());

static const ShaderMapping s_aShadersToLoad[] {
    {shaders::glsl::ntsQuadTexVert, shaders::glsl::ntsQuadTexFrag, "QuadTex"},
    {shaders::glsl::ntsSimpleColorVert, shaders::glsl::ntsSimpleColorFrag, "SimpleColor"},
    {shaders::glsl::ntsSimpleTextureVert, shaders::glsl::ntsSimpleTextureFrag, "SimpleTexture"},
    {shaders::glsl::ntsSkinVert, shaders::glsl::ntsSimpleColorFrag, "Skin"},
    {shaders::glsl::ntsSkinTextureVert, shaders::glsl::ntsInterpolatedColorFrag, "SkinTestColors"},
    {shaders::glsl::ntsSkinTextureVert, shaders::glsl::ntsSimpleTextureFrag, "SkinTexture"},
    {shaders::glsl::ntsSkyboxVert, shaders::glsl::ntsSkyboxFrag, "Skybox"},
};

void
Renderer::init()
{
#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallbackARB(gl::debugCallback, app::g_pWindow);
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    /*glDisable(GL_CULL_FACE);*/

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    s_texDefault = Texture(common::g_spDefaultTexture);

    loadShaders();
    loadAssetObjects();
    loadSkybox();
};

static void
drawNode(const Model& model, const Model::Node& node, const math::M4& trm)
{
    using namespace adt::math;
    const gltf::Node& gltfNode = model.gltfNode(node);
    const auto& win = app::windowInst();
    const f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);
    const auto& camera = control::g_camera;
    const M4 trmProj = M4Pers(toRad(camera.m_fov), aspectRatio, 0.01f, 1000.0f);
    const M4& trmView = camera.m_trm;
    const auto& gltfModel = model.gltfModel();

    for (const int& child : gltfNode.vChildren)
        drawNode(model, model.m_vNodes[child], trm);

    Shader* pSh {};

    auto bindTexture = [&](const gltf::Primitive& primitive) {
        if (primitive.materialI < 0) return;

        auto& mat = gltfModel.m_vMaterials[primitive.materialI];
        if (mat.pbrMetallicRoughness.baseColorTexture.index > -1)
        {
            auto& tex = gltfModel.m_vTextures[mat.pbrMetallicRoughness.baseColorTexture.index];
            auto& img = gltfModel.m_vImages[tex.sourceI];

            Span<char> sp = s_scratch.nextMemZero<char>(img.sUri.size() + 300);
            file::replacePathEnding(&sp, reinterpret_cast<const asset::Object*>(&gltfModel)->m_sMappedWith, img.sUri);

            auto* pObj = asset::search(sp, asset::Object::TYPE::IMAGE);
            ADT_ASSERT(pObj != nullptr, " ");
            auto* pTex = reinterpret_cast<Texture*>(pObj->m_pExtraData);

            if (pTex)
            {
                ADT_ASSERT(pSh != nullptr, " ");
                pTex->bind(GL_TEXTURE0);
            }
        }
        else
        {
            pSh->setV4("u_color", mat.pbrMetallicRoughness.baseColorFactor);
        }

    };

    if (gltfNode.meshI > -1)
    {
        auto& gltfMesh = gltfModel.m_vMeshes[gltfNode.meshI];

        for (const auto& primitive : gltfMesh.vPrimitives)
        {
            /* TODO: there might be any number of TEXCOORD_*,
             * which would be specified in baseColorTexture.texCoord.
             * But current gltf parser only reads the 0th. */
            gltf::Accessor accUV {};
            if (primitive.attributes.TEXCOORD_0 != -1)
                accUV = gltfModel.m_vAccessors[primitive.attributes.TEXCOORD_0];

            if (primitive.attributes.JOINTS_0 != -1)
            {
                ADT_ASSERT(primitive.attributes.WEIGHTS_0 != -1, "must have");

                /*goto GOTO_defaultShader;*/

                if (primitive.materialI > -1 &&
                    gltfModel.m_vMaterials[primitive.materialI].pbrMetallicRoughness.baseColorTexture.index > -1
                )
                {
                    pSh = searchShader("SkinTexture");
                    pSh->use();
                }
                else
                {
                    pSh = searchShader("Skin");
                    pSh->use();
                }

                ADT_ASSERT(gltfNode.skinI > -1, " ");
                const Model::Skin& skin = model.m_vSkins[gltfNode.skinI];
                pSh->setM4("u_model", trm * node.finalTransform);
                pSh->setM4("u_view", trmView);
                pSh->setM4("u_projection", trmProj);
                pSh->setM4("u_a128TrmJoints", Span<M4>(skin.vJointMatrices));

                bindTexture(primitive);
            }
            else if (primitive.materialI != -1)
            {
                auto& mat = gltfModel.m_vMaterials[primitive.materialI];

                if (mat.pbrMetallicRoughness.baseColorTexture.index != -1)
                {
                    auto& tex = gltfModel.m_vTextures[mat.pbrMetallicRoughness.baseColorTexture.index];
                    auto& img = gltfModel.m_vImages[tex.sourceI];

                    Span<char> sp = s_scratch.nextMemZero<char>(img.sUri.size() + 300);
                    file::replacePathEnding(&sp, reinterpret_cast<const asset::Object*>(&gltfModel)->m_sMappedWith, img.sUri);

                    auto* pObj = asset::search(sp, asset::Object::TYPE::IMAGE);

                    if (pObj)
                    {
                        auto* pTex = reinterpret_cast<Texture*>(pObj->m_pExtraData);
                        pSh = searchShader("SimpleTexture");
                        pSh->use();

                        if (pTex) pTex->bind(GL_TEXTURE0);
                        else s_texDefault.bind(GL_TEXTURE0);

                        pSh->setM4("u_trm", trmProj * trmView * trm * node.finalTransform);
                    }
                    else goto GOTO_defaultShader;
                }
                else
                {
                    pSh = searchShader("SimpleColor");
                    pSh->use();
                    pSh->setV4("u_color", mat.pbrMetallicRoughness.baseColorFactor);
                    pSh->setM4("u_trm", trmProj * trmView * trm * node.finalTransform);
                }
            }
            else
            {
GOTO_defaultShader:
                pSh = searchShader("SimpleTexture");
                pSh->use();
                s_texDefault.bind(GL_TEXTURE0);
                pSh->setM4("u_trm", trmProj * trmView * trm * node.finalTransform);
            }

            auto* pPrimitiveData = reinterpret_cast<PrimitiveData*>(primitive.pData);
            if (pPrimitiveData)
            {
                glBindVertexArray(pPrimitiveData->vao);

                if (primitive.indicesI != -1)
                {
                    const gltf::Accessor& accIndices = gltfModel.m_vAccessors[primitive.indicesI];

                    glDrawElements(
                        static_cast<GLenum>(primitive.eMode),
                        accIndices.count,
                        static_cast<GLenum>(accIndices.eComponentType),
                        {}
                    );
                }
                else
                {
                    const auto& accPos = gltfModel.m_vAccessors[primitive.attributes.POSITION];

                    glDrawArrays(
                        static_cast<GLenum>(primitive.eMode),
                        0,
                        accPos.count
                    );
                }
            }
        }
    }
}

static void
drawNodeMesh(const Model& model, const Model::Node& node)
{
    const gltf::Node& gltfNode = model.gltfNode(node);
    const auto& gltfModel = model.gltfModel();

    for (const int& child : gltfNode.vChildren)
        drawNodeMesh(model, model.m_vNodes[child]);

    if (gltfNode.meshI > -1)
    {
        auto& gltfMesh = gltfModel.m_vMeshes[gltfNode.meshI];

        for (const auto& primitive : gltfMesh.vPrimitives)
        {
            auto* pPrimitiveData = reinterpret_cast<PrimitiveData*>(primitive.pData);
            if (pPrimitiveData)
            {
                glBindVertexArray(pPrimitiveData->vao);

                if (primitive.indicesI != -1)
                {
                    const gltf::Accessor& accIndices = gltfModel.m_vAccessors[primitive.indicesI];

                    glDrawElements(
                        static_cast<GLenum>(primitive.eMode),
                        accIndices.count,
                        static_cast<GLenum>(accIndices.eComponentType),
                        {}
                    );
                }
                else
                {
                    const auto& accPos = gltfModel.m_vAccessors[primitive.attributes.POSITION];

                    glDrawArrays(
                        static_cast<GLenum>(primitive.eMode),
                        0,
                        accPos.count
                    );
                }
            }
        }
    }
}

static void
drawModel(const Model& model, math::M4 trm)
{
    const gltf::Model& gltfModel = model.gltfModel();
    const gltf::Scene& scene = gltfModel.m_vScenes[gltfModel.m_defaultSceneI];

    for (const int& nodeI : scene.vNodes)
    {
        const Model::Node& node = model.m_vNodes[nodeI];
        drawNode(model, node, trm);
    }
}

static void
drawModelMesh(const Model& model)
{
    const gltf::Model& gltfModel = model.gltfModel();
    const gltf::Scene& scene = gltfModel.m_vScenes[gltfModel.m_defaultSceneI];

    for (const int& nodeI : scene.vNodes)
    {
        const Model::Node& node = model.m_vNodes[nodeI];
        drawNodeMesh(model, node);
    }
}

static void
drawSkybox()
{
    math::M4 view = control::g_camera.m_trm;
    /* remove translation */
    view.e[3][0] = 0.0f;
    view.e[3][1] = 0.0f;
    view.e[3][2] = 0.0f;

    auto& win = app::windowInst();
    const f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);
    const math::M4 trmProj = math::M4Pers(math::toRad(control::g_camera.m_fov), aspectRatio, 0.01f, 1000.0f);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    Shader* pSh = searchShader("Skybox");
    ADT_ASSERT(pSh, " ");

    if (pSh)
    {
        pSh->use();
        pSh->setM4("u_viewNoTranslate", view);
        pSh->setM4("u_projection", trmProj);
        glBindTexture(GL_TEXTURE_CUBE_MAP, s_skyboxDefault.m_tex);

        Opt<PoolSOAHandle<game::Entity>> oEntity = game::searchEntity("Cube");
        if (oEntity)
        {
            Model& model = Model::fromI(game::g_poolEntities[{oEntity.value().i}].modelI);
            drawModelMesh(model);
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void
Renderer::drawGame([[maybe_unused]] Arena* pArena)
{
    using namespace adt::math;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSkybox();

    {
        auto& entities = game::g_poolEntities;

        /* TODO: implement proper parallel for */
        for (auto& model : Model::g_poolModels)
        {
            s_threadPool.add(+[](void* p) -> THREAD_STATUS
                {
                    reinterpret_cast<Model*>(p)->updateAnimation();
                    return static_cast<THREAD_STATUS>(0);
                }, &model
            );
        }

        for (int entityI = entities.firstI();
            entityI < entities.m_size;
            entityI = entities.nextI(entityI)
        )
        {
            if (game::g_poolEntities.bindMember<&game::Entity::bNoDraw>({entityI}))
                continue;

            game::EntityBind entity = game::g_poolEntities[{entityI}];
            auto& obj = asset::g_poolObjects[{entity.assetI}];

            switch (obj.m_eType)
            {
                default: break;

                case asset::Object::TYPE::MODEL:
                {
                    Model& model = Model::fromI(entity.modelI);

                    drawModel(model,
                        M4TranslationFrom(entity.pos) *
                        QtRot(entity.rot) *
                        M4ScaleFrom(entity.scale)
                    );
                }
                break;
            }
        }
    }
}

void
Renderer::destroy()
{
    s_threadPool.destroy();

    for (Shader& shader : g_poolShaders)
        shader.destroy();
}

ShaderMapping::ShaderMapping(const StringView svVert, const StringView svFrag, const StringView svMappedTo)
    : m_svVert(svVert), m_svFrag(svFrag), m_svMappedTo(svMappedTo), m_eType(TYPE::VS_FS) {}

Texture::Texture(int width, int height)
    : m_width(width), m_height(height)
{
    loadRGBA(nullptr);
}

Texture::Texture(const adt::Span2D<ImagePixelRGBA> spImg)
    : m_width(spImg.width()), m_height(spImg.height())
{
    loadRGBA(spImg.data());
}

void
Texture::subImage(const Span2D<ImagePixelRGBA> spImg)
{
    ADT_ASSERT(
        spImg.stride() > 0 && m_width <= spImg.stride() &&
        spImg.height() > 0 && m_height <= spImg.height(),
        "invalid spImg size"
    );

    bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, spImg.stride(), spImg.height(), GL_RGBA, GL_UNSIGNED_BYTE, spImg.data());
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

Shader::Shader(const adt::StringView svVertexShader, const adt::StringView svFragmentShader, const adt::StringView svMapTo)
    : m_svMappedTo(svMapTo)
{
    GLint linked {};
    GLuint vertex = loadOne(GL_VERTEX_SHADER, svVertexShader);
    GLuint fragment = loadOne(GL_FRAGMENT_SHADER, svFragmentShader);

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

    s_mapStringToShaders.insert(svMapTo, g_poolShaders.make(*this));
}

GLuint
Shader::loadOne(GLenum type, adt::StringView sShader)
{
    GLuint shader = glCreateShader(type);
    if (!shader) return 0;

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
            GLsizei len {};
            glGetShaderInfoLog(shader, infoLen, &len, aBuff);
            LOG_BAD("error compiling shader:\n{}\n", StringView{aBuff, len});
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
        StringView typeName {};

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
    s_mapStringToShaders.remove(m_svMappedTo);
    LOG_NOTIFY("shader {} '{}' destroyed\n", m_id, m_svMappedTo);
    *this = {};
}

Quad::Quad(adt::InitFlag)
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
searchShader(const adt::StringView svKey)
{
    auto res = s_mapStringToShaders.search(svKey);
    if (!res)
    {
        ADT_ASSERT(false, "'%.*s' not found", static_cast<int>(svKey.size()), svKey.data());
        return {};
    }

    return &g_poolShaders[res.value()];
}

static void
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

        case gltf::COMPONENT_TYPE::INT:
        return static_cast<int>(sizeof(GLint));

        case gltf::COMPONENT_TYPE::FLOAT:
        return static_cast<int>(sizeof(GLfloat));
    }

    char aBuff[127] {};
    print::toSpan(aBuff, "invalid component type: '{}'", eType);
    ADT_ASSERT(false, "%s", aBuff);
    return 0;
}

template<typename A, typename B>
static void
bufferViewConvert(
    const View<A> spA,
    const int accessorCount,
    const int attrLocation,
    GLint size,
    GLenum eType,
    GLuint* pVbo
)
{
    ADT_ASSERT(pVbo != nullptr, " ");

    Span<B> spB(StdAllocator::inst()->zallocV<B>(accessorCount), accessorCount);
    defer( StdAllocator::inst()->free(spB.data()) );
    ADT_ASSERT(spB.size() == accessorCount, "sp.size: %lld, acc.count: %d", spB.size(), accessorCount);

    ssize maxSize = utils::min(spB.size(), spA.size());
    for (ssize spI = 0; spI < maxSize; ++spI)
    {
        auto& elementA = spA[spI];
        spB[spI] = B(elementA);
    }

    glGenBuffers(1, pVbo);
    glBindBuffer(GL_ARRAY_BUFFER, *pVbo);
    glBufferData(GL_ARRAY_BUFFER, spB.size()*sizeof(*spB.data()), spB.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(attrLocation);
    glVertexAttribPointer(
        attrLocation, /* enabled index */
        size,
        eType,
        false,
        0,
        0
    );
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

    obj.m_pExtraData = obj.m_arena.alloc<Texture>(pImage->spanRGBA());
}

static void
loadGLTF(gltf::Model* pModel)
{
    if (!pModel)
    {
        LOG_WARN("pModel: {}\n", pModel);
        return;
    }

    auto& obj = *reinterpret_cast<asset::Object*>(pModel);

    VecManaged<GLuint> vVBOs(StdAllocator::inst());;
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
    if (vVBOs.empty()) return;

    {
        for (auto& mesh : pModel->m_vMeshes)
        {
            LOG_GOOD("loading mesh: '{}'...\n", mesh.sName);
            for (auto& primitive : mesh.vPrimitives)
            {
                PrimitiveData newPrimitiveData {};

                glGenVertexArrays(1, &newPrimitiveData.vao);
                glBindVertexArray(newPrimitiveData.vao);
                defer( glBindVertexArray(0) );

                if (primitive.indicesI != -1)
                {
                    auto& accInd = pModel->m_vAccessors[primitive.indicesI];
                    auto& viewInd = pModel->m_vBufferViews[accInd.bufferViewI];
                    auto& buffInd = pModel->m_vBuffers[viewInd.bufferI];

                    {
                        /* NOTE: we are duplicating index buffer here */
                        glGenBuffers(1, &newPrimitiveData.ebo);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newPrimitiveData.ebo);
                        glBufferData(
                            GL_ELEMENT_ARRAY_BUFFER,
                            viewInd.byteLength,
                            &buffInd.sBin[accInd.byteOffset + viewInd.byteOffset],
                            GL_STATIC_DRAW
                        );
                    }
                }

                /* positions */
                {
                    ADT_ASSERT(primitive.attributes.POSITION != -1, " ");
                    auto& accPos = pModel->m_vAccessors[primitive.attributes.POSITION];
                    auto& viewPos = pModel->m_vBufferViews[accPos.bufferViewI];
                    auto& buffPos = pModel->m_vBufferViews[viewPos.bufferI];

                    newPrimitiveData.vbo = vVBOs[buffPos.bufferI];
                    glBindBuffer(GL_ARRAY_BUFFER, newPrimitiveData.vbo);

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

                /* NOTE:
                 * JOINTS_n: unsigned byte or unsigned short
                 * WEIGHTS_n: float, or normalized unsigned byte, or normalized unsigned short */

                /* joints */
                if (primitive.attributes.JOINTS_0 != -1)
                {
                    gltf::Accessor accJoints = pModel->m_vAccessors[primitive.attributes.JOINTS_0];

                    switch (accJoints.eComponentType)
                    {
                        default: LOG_BAD("unexpected component type\n"); break;

                        case gltf::COMPONENT_TYPE::UNSIGNED_BYTE:
                        {
                            View<math::IV4u8> vwU8(pModel->accessorView<math::IV4u8>(primitive.attributes.JOINTS_0));

                            bufferViewConvert<math::IV4u8, math::V4>(
                                vwU8, accJoints.count, shaders::glsl::JOINT_LOCATION, 4, GL_FLOAT, &newPrimitiveData.vboJoints
                            );
                        }
                        break;

                        case gltf::COMPONENT_TYPE::UNSIGNED_SHORT:
                        {
                            View<math::IV4u16> vwU16(pModel->accessorView<math::IV4u16>(primitive.attributes.JOINTS_0));

                            bufferViewConvert<math::IV4u16, math::V4>(
                                vwU16, accJoints.count, shaders::glsl::JOINT_LOCATION, 4, GL_FLOAT, &newPrimitiveData.vboJoints
                            );
                        }
                        break;
                    }
                }

                /* weights */
                if (primitive.attributes.JOINTS_0 != -1 && primitive.attributes.WEIGHTS_0 == -1)
                {
                    LOG_BAD("Skinned nodes must contain WEIGHTS_*\n");
                }
                else if (primitive.attributes.JOINTS_0 != -1 && primitive.attributes.WEIGHTS_0 != -1)
                {
                    gltf::Accessor accWeights = pModel->m_vAccessors[primitive.attributes.WEIGHTS_0];

                    switch (accWeights.eComponentType)
                    {
                        default: LOG_BAD("unhandled component type\n"); break;

                        case gltf::COMPONENT_TYPE::UNSIGNED_BYTE:
                        {
                        }
                        break;

                        case gltf::COMPONENT_TYPE::UNSIGNED_SHORT:
                        {
                            const View<math::IV4u16> vwU16(pModel->accessorView<math::IV4u16>(primitive.attributes.WEIGHTS_0));

                            bufferViewConvert<math::IV4u16, math::V4>(
                                vwU16, accWeights.count, shaders::glsl::WEIGHT_LOCATION, 4, GL_FLOAT, &newPrimitiveData.vboWeights
                            );
                        }
                        break;

                        case gltf::COMPONENT_TYPE::FLOAT:
                        {
                            const View<math::V4> vwV4(pModel->accessorView<math::V4>(primitive.attributes.WEIGHTS_0));

                            bufferViewConvert<math::V4, math::V4>(
                                vwV4, accWeights.count,
                                shaders::glsl::WEIGHT_LOCATION, 4, static_cast<GLenum>(accWeights.eComponentType),
                                &newPrimitiveData.vboWeights
                            );
                        }
                        break;
                    }
                }

                primitive.pData = obj.m_arena.alloc<PrimitiveData>(newPrimitiveData);
            }
        }
    }
}

static void
loadAssetObjects()
{
    if (asset::g_poolObjects.empty())
        LOG_WARN("asset::g_aObjects.empty(): {}\n", asset::g_poolObjects.empty());

    for (auto& obj : asset::g_poolObjects)
    {
        switch (obj.m_eType)
        {
            case asset::Object::TYPE::MODEL:
            loadGLTF(&obj.m_uData.model);
            break;

            case asset::Object::TYPE::IMAGE:
            loadImage(&obj.m_uData.img);
            break;

            case asset::Object::TYPE::NONE:
            break;
        }
    }
}

static void
loadSkybox()
{
    Image* i0 = asset::searchImage("assets/skybox/right.bmp");
    Image* i1 = asset::searchImage("assets/skybox/left.bmp");
    Image* i2 = asset::searchImage("assets/skybox/top.bmp");
    Image* i3 = asset::searchImage("assets/skybox/bottom.bmp");
    Image* i4 = asset::searchImage("assets/skybox/front.bmp");
    Image* i5 = asset::searchImage("assets/skybox/back.bmp");

    Image a6Images[6] {};

    if (!i0 || !i1 || !i2 || !i3 || !i4 || !i5)
    {
        LOG_BAD("failed to load skybox, using default texture\n");

        Image img {
            .m_uData {.pRGBA = const_cast<ImagePixelRGBA*>(common::g_spDefaultTexture.data())},
            .m_width = static_cast<i16>(common::g_spDefaultTexture.width()),
            .m_height = static_cast<i16>(common::g_spDefaultTexture.height()),
            .m_eType = Image::TYPE::RGBA,
        };

        for (auto& el : a6Images) el = img;
    }
    else
    {
        a6Images[0] = *i0;
        a6Images[1] = *i1;
        a6Images[2] = *i2;
        a6Images[3] = *i3;
        a6Images[4] = *i4;
        a6Images[5] = *i5;
    }

    s_skyboxDefault = Skybox(a6Images);
}

Skybox::Skybox(Image a6Images[6])
{
    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

    for (ssize i = 0; i < 6; ++i)
    {
        Image& img = a6Images[i];

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGBA, img.m_width, img.m_height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, img.m_uData.pRGBA);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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
