#include "gl.hh"

#include "adt/Map.hh"
#include "adt/Opt.hh"
#include "adt/OsAllocator.hh"
#include "adt/file.hh"
#include "adt/logs.hh"

using namespace adt;

namespace gl
{

Pool<Shader, 128> g_shaders(adt::INIT);
static Map<String, PoolHnd> s_mapStringToShaders(OsAllocatorGet(), g_shaders.getCap());

static const ShaderMapping s_aShadersToLoad[] {
    {"src/shaders/quad/tex.vert", "src/shaders/quad/tex.frag", "Quad"},
};

ShaderMapping::ShaderMapping(const String svVertPath, const String svFragPath, const String svMappedTo)
    : m_svMappedTo(svMappedTo), m_eType(TYPE::VS_FS)
{
    m_uPaths.vsFs.svVert = svVertPath;
    m_uPaths.vsFs.svFrag = svFragPath;
}

Texture::Texture(int width, int height)
    : m_width(width), m_height(height)
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
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

    s_mapStringToShaders.insert(svMapTo, g_shaders.push(*this));
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

    return &g_shaders[res.value()];
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
                OsAllocator al;

                Opt<String> osVert = file::load(&al, shader.m_uPaths.vsFs.svVert);
                if (!osVert)
                    exit(1);
                defer( osVert.value().destroy(&al) );

                Opt<String> osFrag = file::load(&al, shader.m_uPaths.vsFs.svFrag);
                if (!osFrag)
                    exit(1);
                defer( osFrag.value().destroy(&al) );

                /* maps to gl::g_shaders */
                gl::Shader(osVert.value(), osFrag.value(), shader.m_svMappedTo);
            }
            break;

            case ShaderMapping::TYPE::VS_GS_FS:
            ADT_ASSERT(false, "TODO");
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

} /* namespace gl */
