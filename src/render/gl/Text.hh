#pragma once

#include "gl.hh" /* IWYU pragma: keep */
#include "ttf/Rasterizer.hh"

namespace render::gl
{

struct CharQuad2Pos2UV
{
    adt::Pair<adt::math::V2, adt::math::V2> a[6];
};

struct Text
{
    GLuint m_vao {};
    GLuint m_vbo {};
    GLuint m_vboSize {};
    GLuint m_texId {};
    int m_maxSize {};

    /* */

    Text() = default;
    Text(const int maxSize);

    /* */

    void update(const ttf::Rasterizer& rast, const adt::StringView sv);
    void bind() const { glBindVertexArray(m_vao); }
    void draw() const { glDrawArrays(GL_TRIANGLES, 0, m_vboSize); }
    void bindDraw() const { bind(); draw(); }

protected:
    [[nodiscard]] adt::Vec<CharQuad2Pos2UV> makeStringMesh(
        const ttf::Rasterizer& rast, const adt::StringView sv
    );
};

} /* namespace render::gl */
