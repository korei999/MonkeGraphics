#pragma once

#include "glfunc.hh" /* IWYU pragma: keep */
#include "../IRenderer.hh"

#include "adt/math.hh"
#include "adt/Pool.hh"

#include "Image.hh"

namespace render::gl
{

struct Renderer : public IRenderer
{
    virtual void init() override;
    virtual void draw(adt::Arena* pArena) override;
    virtual void destroy() override;
};

struct ShaderMapping
{
    enum class TYPE : adt::u8 { VS_FS, VS_GS_FS };

    /* */

    adt::StringView m_svVert {};
    adt::StringView m_svGeom {};
    adt::StringView m_svFrag {};
    adt::StringView m_svMappedTo {};

    TYPE m_eType {};

    /* */

    ShaderMapping() = default;
    ShaderMapping(const adt::StringView svVertPath, const adt::StringView svFragPath, const adt::StringView svMappedTo);
};

struct Texture
{
    GLuint m_id {};
    int m_width {};
    int m_height {};
    Image::TYPE m_eType {};

    /* */

    Texture() = default;
    [[nodiscard]] Texture(int width, int height);
    [[nodiscard]] Texture(const adt::Span2D<ImagePixelRGBA> spImg);
    [[nodiscard]] Texture(const adt::Span2D<adt::u8> spImgMono, GLint minManParam = GL_LINEAR);

    /* */

    void bind() { glBindTexture(GL_TEXTURE_2D, m_id); }
    void bind(GLint activeTexture) { glActiveTexture(activeTexture); bind(); }
    void subImage(const adt::Span2D<ImagePixelRGBA> spImg);
    void destroy();

private:
    void loadRGBA(const ImagePixelRGBA* pData);
};

struct Shader
{
    GLuint m_id = 0; /* gl shader handle */
    adt::StringView m_svMappedTo {};

    /* */

    Shader() = default;
    Shader(const adt::StringView svVertexShader, const adt::StringView svFragmentShader, const adt::StringView svMapTo);

    /* */

    void queryActiveUniforms();
    void destroy();

    void use() const { glUseProgram(m_id); }
    
    void 
    setM3(const char* ntsUnifromVar, const adt::math::M3& m)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUnifromVar);
        glUniformMatrix3fv(ul, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(m.data()));
    }
    
    void 
    setM4(const char* ntsUnifromVar, const adt::math::M4& m)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUnifromVar);
        glUniformMatrix4fv(ul, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(m.data()));
    }

    void 
    setM4(const char* ntsUnifromVar, const adt::Span<adt::math::M4> sp)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUnifromVar);
        glUniformMatrix4fv(ul, sp.size(), GL_FALSE, reinterpret_cast<const GLfloat*>(sp.data()));
    }

    void
    setV2(const char* ntsUnifromVar, const adt::math::V2& v)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUnifromVar);
        glUniform2fv(ul, 1, reinterpret_cast<const GLfloat*>(v.data()));
    }
    
    void
    setV3(const char* ntsUnifromVar, const adt::math::V3& v)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUnifromVar);
        glUniform3fv(ul, 1, v.data());
    }
    
    void
    setV4(const char* ntsUnifromVar, const adt::math::V4& v)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUnifromVar);
        glUniform4fv(ul, 1, v.data());
    }
    
    void
    setI(const char* ntsUniformVar, const GLint i)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUniformVar);
        glUniform1i(ul, i);
    }
    
    void
    setF(const char* ntsUnifromVar, const adt::f32 f)
    {
        GLint ul = glGetUniformLocation(m_id, ntsUnifromVar);
        glUniform1f(ul, f);
    }

private:
    void load(const adt::StringView svVertexPath, const adt::StringView svFragmentPath);
    static GLuint loadOne(GLenum type, adt::StringView path);
};

struct Quad
{
    enum class TYPE : adt::u8 { MINUS_ONE_TO_ONE, ZERO_TO_ONE };

    static constexpr adt::f32 aMINUS_ONE_TO_ONE[24] {
        /* pos(0, 1)      uv(2, 3) */
        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
        -1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f,  1.0f,    1.0f, 1.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
    };

    static constexpr adt::f32 aZERO_TO_ONE[24] {
        /* pos(0, 1)      uv(2, 3) */
        0.0f,  1.0f,    0.0f, 1.0f,
        1.0f,  0.0f,    1.0f, 0.0f,
        0.0f,  0.0f,    0.0f, 0.0f,

        0.0f,  1.0f,    0.0f, 1.0f,
        1.0f,  1.0f,    1.0f, 1.0f,
        1.0f,  0.0f,    1.0f, 0.0f,
    };

    /* */

    GLuint m_vao {};
    GLuint m_vbo {};

    /* */

    Quad() = default;
    Quad(adt::InitFlag, TYPE eType = TYPE::MINUS_ONE_TO_ONE);

    /* */

    void bind() { glBindVertexArray(m_vao); }
    void draw() { glDrawArrays(GL_TRIANGLES, 0, 6); }
    void bindDraw() { bind(); draw(); }
};

[[nodiscard]] Shader* searchShader(const adt::StringView svKey);

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

struct Text;

extern adt::Pool<Shader, 128> g_poolShaders;
extern Quad g_quad;
extern Texture g_texDefault;
extern Shader* g_pShColor;

} /* namespace render::gl */
