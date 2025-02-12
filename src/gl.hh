#pragma once

#include "platform/glfunc.hh"

#include "adt/String.hh"
#include "adt/math.hh"
#include "adt/Pool.hh"

#include "Image.hh"

namespace gl
{

struct ShaderMapping
{
    enum class TYPE : adt::u8 { VS_FS, VS_GS_FS };

    /* */

    union
    {
        struct
        {
            adt::String svVert {};
            adt::String svFrag {};
        } vsFs;

        struct
        {
            adt::String svVert {};
            adt::String svGeom {};
            adt::String svFrag {};
        } vsGsFs;
    } m_uPaths {};
    adt::String m_svMappedTo {};
    TYPE m_eType {};

    /* */

    ShaderMapping() = default;
    ShaderMapping(const adt::String svVertPath, const adt::String svFragPath, const adt::String svMappedTo);
};

struct Texture
{
    GLuint m_texture {};
    int m_width {};
    int m_height {};

    /* */

    Texture() = default;

    Texture(int width, int height);

    /* */

    void bind() { glBindTexture(GL_TEXTURE_2D, m_texture); }
    void bind(GLint activeTexture) { glActiveTexture(activeTexture); bind(); }
    void subImage(const adt::Span2D<ImagePixelRGBA> spImg);
    void destroy();
};

struct Shader
{
    GLuint m_id = 0; /* gl shader handle */

    /* */

    Shader() = default;
    Shader(const adt::String sVertexShader, const adt::String sFragmentShader, const adt::String svMapTo);

    /* */

    void queryActiveUniforms();
    void destroy();

    void use() const { glUseProgram(m_id); }
    
    void 
    setM3(adt::String name, const adt::math::M3& m)
    {
        GLint ul = glGetUniformLocation(m_id, name.data());
        glUniformMatrix3fv(ul, 1, GL_FALSE, (GLfloat*)m.e);
    }
    
    void 
    setM4(adt::String name, const adt::math::M4& m)
    {
        GLint ul = glGetUniformLocation(m_id, name.data());
        glUniformMatrix4fv(ul, 1, GL_FALSE, (GLfloat*)m.e);
    }
    
    void
    setV3(adt::String name, const adt::math::V3& v)
    {
        GLint ul = glGetUniformLocation(m_id, name.data());
        glUniform3fv(ul, 1, (GLfloat*)v.e);
    }
    
    void
    setV4(adt::String name, const adt::math::V4& v)
    {
        GLint ul = glGetUniformLocation(m_id, name.data());
        glUniform4fv(ul, 1, (GLfloat*)v.e);
    }
    
    void
    setI(const char* ntsUniform, const GLint i)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUniform);
        glUniform1i(ul, i);
    }
    
    void
    setF(adt::String name, const adt::f32 f)
    {
        GLint ul = glGetUniformLocation(m_id, name.data());
        glUniform1f(ul, f);
    }

private:
    void load(const adt::String svVertexPath, const adt::String svFragmentPath);
    static GLuint loadOne(GLenum type, adt::String path);
};

struct Quad
{
    GLuint m_vao {};
    GLuint m_vbo {};

    /* */

    Quad() = default;
    Quad(adt::INIT_FLAG);

    /* */

    void bind() { glBindVertexArray(m_vao); }
    void draw() { glDrawArrays(GL_TRIANGLES, 0, 6); }
};

[[nodiscard]] Shader* searchShader(const adt::String svKey);
void init();
void loadShaders();

#ifndef NDEBUG
void debugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* user
);
#endif

extern adt::Pool<Shader, 128> g_shaders;

} /* namespace gl */
