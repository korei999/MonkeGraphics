#include "Text.hh"

#include "app.hh"
#include "adt/BufferAllocator.hh"

using namespace adt;

namespace render::gl
{

Vec<CharQuad2Pos2UV>
Text::makeStringMesh(
    const ttf::Rasterizer& rast,
    const StringView vs,
    const bool bVerticalFlip
)
{
    auto spMem = app::gtl_scratch.nextMem<CharQuad2Pos2UV>(m_maxSize);
    if (spMem.size() < m_maxSize) return {};

    /* NOTE: problems with constructor */
    BufferAllocator al((u8*)spMem.data(), spMem.size() * sizeof(spMem[0]));

    Vec<CharQuad2Pos2UV> vQuads(&al, m_maxSize);
    vQuads.setSize(&al, m_maxSize);

    f32 xOff = 0.0f;
    f32 yOff = 0.0f;

    const f32 norm = 1.0f / rast.m_altas.m_width;
    const f32 yCoordOff = norm * rast.m_scale;
    const f32 xCoordOff = (norm * rast.m_scale) * rast.X_STEP;

#define SKIP { xOff += 1.0f; continue; }

    const f32 addY = bVerticalFlip ? +1.0f : -1.0f;

    for (const char& ch : vs)
    {
        if (ch == ' ')
        {
            SKIP;
        }
        else if (ch == '\n')
        {
            xOff = 0.0f;
            yOff += addY;
            continue;
        }

        ssize idx = vs.idx(&ch);
        if (idx >= m_maxSize) break;

        const MapResult fCh = rast.m_mapCodeToUV.search(ch);

        if (!fCh) SKIP;

#undef SKIP

        const Pair<i16, i16> uv = fCh.value();
        const f32 u = uv.first * norm;
        const f32 v = uv.second * norm;

        /* tl */
        f32 x0;
        f32 y0;
        /* tr */
        f32 x1;
        f32 y1;
        /* bl */
        f32 x2;
        f32 y2;
        /* br */
        f32 x3;
        f32 y3;

        if (!bVerticalFlip)
        {
            /* tl */
            x0 = u;
            y0 = v + yCoordOff;

            /* tr */
            x1 = u + xCoordOff;
            y1 = v + yCoordOff;

            /* bl */
            x2 = u;
            y2 = v;

            /* br */
            x3 = u + xCoordOff;
            y3 = v;
        }
        else
        {
            /* tl (flipped) */
            x0 = u;
            y0 = v;

            /* tr (flipped) */
            x1 = u + xCoordOff;
            y1 = v;

            /* bl (flipped) */
            x2 = u;
            y2 = v + yCoordOff;

            /* br (flipped) */
            x3 = u + xCoordOff;
            y3 = v + yCoordOff;
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
Text::update(const ttf::Rasterizer& rast, const StringView sv, const bool bVerticalFlip)
{
    /* construct from gtl_scratch */
    Vec<CharQuad2Pos2UV> vQuads = makeStringMesh(rast, sv, bVerticalFlip);
    m_vboSize = vQuads.size() * 6; /* 6 vertices for 1 quad */

    if (m_vboSize > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glBufferSubData(GL_ARRAY_BUFFER, 0,
            utils::min(vQuads.size(), static_cast<ssize>(m_maxSize)) * sizeof(vQuads[0]),
            vQuads.data()
        );
    }
}

} /* namespace render::gl */
