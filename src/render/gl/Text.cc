#include "Text.hh"

using namespace adt;

namespace render::gl
{

Vec<CharQuad2Pos2UV>
Text::makeStringMesh(
    Arena* pArena,
    const ttf::Rasterizer& rast,
    const StringView vs
)
{
    Vec<CharQuad2Pos2UV> vQuads(pArena, m_maxSize);
    vQuads.setSize(pArena, m_maxSize);

    f32 xOff = 0.0f;
    f32 yOff = 0.0f;

    const f32 norm = 1.0f / rast.m_altas.m_width;
    const f32 yStep = norm * rast.m_scale;
    const f32 xStep = (norm * rast.m_scale) * rast.X_STEP;

#define SKIP { xOff += 1.0f; continue; }

    for (const char& ch : vs)
    {
        if (ch == ' ')
            SKIP;

        ssize idx = vs.idx(&ch);
        if (idx >= m_maxSize) break;

        const MapResult fCh = rast.m_mapCodeToXY.search(ch);

        if (!fCh)
            SKIP;

#undef SKIP

        const Pair<i16, i16> uv = fCh.value();
        const f32 u = uv.first * norm;
        const f32 v = uv.second * norm;

        /* tl */
        f32 x0 = u;
        f32 y0 = v + yStep;

        /* tr */
        f32 x1 = u + xStep;
        f32 y1 = v + yStep;

        /* bl */
        f32 x2 = u;
        f32 y2 = v;

        /* br */
        f32 x3 = u + xStep;
        f32 y3 = v;

        if (ch == '\n')
        {
            xOff = 0.0f;
            yOff += 2.0f;
            continue;
        }

        vQuads[idx] = {
             0.0f + xOff, 1.0f + yOff, x0, y0,
             1.0f + xOff, 0.0f + yOff, x3, y3,
             0.0f + xOff, 0.0f + yOff, x2, y2,

             0.0f + xOff, 1.0f + yOff, x0, y0,
             1.0f + xOff, 1.0f + yOff, x1, y1,
             1.0f + xOff, 0.0f + yOff, x3, y3,
        };

        xOff += 1.0f;
    }

    return vQuads;
}

Text::Text(const int maxSize)
    : m_maxSize(maxSize)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    defer( glBindVertexArray(0) );

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_maxSize * sizeof(CharQuad2Pos2UV), nullptr, GL_DYNAMIC_DRAW);

    /* positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32)*4, (void*)0); /* 2 pos 2 uv */
    /* texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32)*4, (void*)(sizeof(math::V2)));
}

void
Text::update(Arena* pArena, const ttf::Rasterizer& rast, const StringView sv)
{
    Vec<CharQuad2Pos2UV> vQuads = makeStringMesh(pArena, rast, sv);
    m_vboSize = vQuads.size() * 6; /* 6 vertices for 1 quad */

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0,
        utils::min(vQuads.size(), static_cast<ssize>(m_maxSize)) * sizeof(vQuads[0]),
        vQuads.data()
    );
}

} /* namespace render::gl */
