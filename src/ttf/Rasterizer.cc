#include "Rasterizer.hh"

#include "Font.hh"

using namespace adt;

namespace ttf
{

struct CurveEndIdx { u16 aIdxs[8]; };

struct PointOnCurve
{
    math::V2 pos;
    bool bOnCurve;
    bool bEndOfCurve;
    f32 _pad {};
};

Vec<PointOnCurve>
pointsWithMissingOnCurve(IAllocator* pAlloc, Glyph* g)
{
    const auto& aGlyphPoints = g->uGlyph.simple.vPoints;
    u32 size = aGlyphPoints.size();

    bool bCurrOnCurve = false;
    bool bPrevOnCurve = false;
    u32 firstInCurveIdx = 0;

    Vec<PointOnCurve> aPoints(pAlloc, size);
    int nOffCurve = 0;
    for (const auto& p : aGlyphPoints)
    {
        const u32 pointIdx = aGlyphPoints.idx(&p);

        f32 x = f32(p.x);
        f32 y = f32(p.y);

        bool bEndOfCurve = false;
        for (auto e : g->uGlyph.simple.vEndPtsOfContours)
            if (e == pointIdx)
                bEndOfCurve = true;

        math::V2 vCurr {x, y};

        bCurrOnCurve = p.bOnCurve;
        defer( bPrevOnCurve = bCurrOnCurve );

        if (!bCurrOnCurve && !bPrevOnCurve)
        {
            /* insert middle point */
            const auto& prev = aPoints.last();
            math::V2 mid = math::lerp(prev.pos, vCurr, 0.5f);

            aPoints.push(pAlloc, {
                .pos = mid,
                .bOnCurve = true,
                .bEndOfCurve = false
            });
        }

        aPoints.push(pAlloc, {
            .pos {x, y},
            .bOnCurve = p.bOnCurve,
            .bEndOfCurve = bEndOfCurve
        });

        if (!bCurrOnCurve && bEndOfCurve)
            assert(aGlyphPoints[firstInCurveIdx].bOnCurve == true);

        if (bEndOfCurve) firstInCurveIdx = pointIdx + 1;
    }

    return aPoints;
}

static void
insertPoints(
    IAllocator* pAlloc,
    Vec<PointOnCurve>* aPoints,
    const math::V2& p0,
    const math::V2& p1,
    const math::V2& p2,
    int nTimes = 1
)
{
    for (int i = 1; i < nTimes + 1; ++i)
    {
        f32 t = f32(i) / f32(nTimes + 1);

        auto point = math::bezier(p0, p1, p2, t);
        aPoints->push(pAlloc, {
            .pos = point,
            .bOnCurve = true,
            .bEndOfCurve = false
        });
    }
}

static Vec<PointOnCurve>
makeItCurvy(IAllocator* pAlloc, const Vec<PointOnCurve>& aNonCurvyPoints, CurveEndIdx* pEndIdxs, u32 nTessellations)
{
    Vec<PointOnCurve> aNew(pAlloc, aNonCurvyPoints.size());
    utils::fill(pEndIdxs->aIdxs, NPOS16, utils::size(pEndIdxs->aIdxs));
    u16 endIdx = 0;

    u32 firstInCurveIdx = 0;
    bool bPrevOnCurve = true;
    for (auto& p : aNonCurvyPoints)
    {
        u32 idx = aNonCurvyPoints.idx(&p);

        if (p.bEndOfCurve)
        {
            if (!aNonCurvyPoints[idx].bOnCurve || !aNonCurvyPoints[firstInCurveIdx].bOnCurve)
            {
                if (nTessellations > 0)
                {
                    math::V2 p0 {aNonCurvyPoints[idx - 1].pos};
                    math::V2 p1 {aNonCurvyPoints[idx - 0].pos};
                    math::V2 p2 {aNonCurvyPoints[firstInCurveIdx].pos};
                    insertPoints(pAlloc, &aNew, p0, p1, p2, nTessellations);
                }
            }
        }

        if (!bPrevOnCurve)
        {
            if (nTessellations > 0)
            {
                math::V2 p0 {aNonCurvyPoints[idx - 2].pos};
                math::V2 p1 {aNonCurvyPoints[idx - 1].pos};
                math::V2 p2 {aNonCurvyPoints[idx - 0].pos};
                insertPoints(pAlloc, &aNew, p0, p1, p2, nTessellations);
            }
        }

        if (p.bOnCurve)
        {
            aNew.push(pAlloc, {
                .pos = p.pos,
                .bOnCurve = p.bOnCurve,
                .bEndOfCurve = false
            });
        }

        if (p.bEndOfCurve)
        {
            aNew.push(pAlloc, {
                .pos = aNonCurvyPoints[firstInCurveIdx].pos,
                .bOnCurve = true,
                .bEndOfCurve = true,
            });

            if (endIdx < 8) pEndIdxs->aIdxs[endIdx++] = aNew.lastI();
            else assert(false && "8 curves max");

            firstInCurveIdx = idx + 1;
            bPrevOnCurve = true;
        }
        else
        {
            bPrevOnCurve = p.bOnCurve;
        }
    }

    return aNew;
}

void
Rasterizer::rasterizeGlyph(Arena* pArena, Font* pFont, Glyph* pGlyph, int xOff, int yOff)
{
    const Vec<Point>& vGlyphPoints = pGlyph->uGlyph.simple.vPoints;

    CurveEndIdx endIdxs;
    Vec<PointOnCurve> vCurvyPoints = makeItCurvy(
        pArena, pointsWithMissingOnCurve(pArena, pGlyph), &endIdxs, 6
    );

    f32 xMax = pFont->m_head.xMax;
    f32 xMin = pFont->m_head.xMin;
    f32 yMax = pFont->m_head.yMax;
    f32 yMin = pFont->m_head.yMin;

    Array<f32, 64> aIntersections {};
    Span2D<u8> sp = m_altas.spanMono();

    const f32 hScale = static_cast<f32>(m_scale) / static_cast<f32>(xMax - xMin);
    const f32 vScale = static_cast<f32>(m_scale) / static_cast<f32>(yMax - yMin);

    for (ssize row = 0; row < static_cast<ssize>(m_scale); ++row)
    {
        aIntersections.setSize(0);
        const f32 scanline = static_cast<f32>(row);

        for (ssize pointIdx = 1; pointIdx < vCurvyPoints.size(); ++pointIdx)
        {
            f32 x0 = (vCurvyPoints[pointIdx - 1].pos.x) * hScale;
            f32 x1 = (vCurvyPoints[pointIdx - 0].pos.x) * hScale;

            f32 y0 = (vCurvyPoints[pointIdx - 1].pos.y - yMin) * vScale;
            f32 y1 = (vCurvyPoints[pointIdx - 0].pos.y - yMin) * vScale;

            if (vCurvyPoints[pointIdx].bEndOfCurve)
                ++pointIdx;

            /* for the intersection all we need is to find what X is when our y = scanline or when y is equal to i of the loop
             *
             * y - y1 = m*(x - x1) sub both sides by m
             * (y - y1)/m = x - x1 add x1 to both sides
             * (y-y1)*1/m + x1 = x m is just the slope of the line so dy/dx and 1/m is dx/dy
             * y in this equation would be the scanline, x1 & y1 */

            const f32 biggerY = utils::max(y0, y1);
            const f32 smallerY = utils::min(y0, y1);

            if (scanline <= smallerY || scanline > biggerY) continue;

            const f32 dx = x1 - x0;
            const f32 dy = y1 - y0;

            if (math::eq(dy, 0.0f)) continue;

            f32 intersection = -1.0f;

            if (math::eq(dx, 0.0f))
                intersection = x1;
            else intersection = (scanline - y1)*(dx/dy) + x1;

            if (aIntersections.size() >= aIntersections.cap()) continue;
            aIntersections.push(intersection);
        }

        if (aIntersections.size() > 1)
        {
            sort::insertion(&aIntersections);

            for (ssize intIdx = 0; intIdx < aIntersections.size(); intIdx += 2)
            {
                f32 start = aIntersections[intIdx];
                int startIdx = start;
                f32 startCovered = (startIdx + 1) - start;

                f32 end = aIntersections[intIdx + 1];
                int endIdx = end;
                f32 endCovered = end - endIdx;

                /*if (startIdx >= 0)*/
                /*    sp(xOff + startIdx, yOff + row) = 255.0f * startCovered;*/
                /*if (startIdx != endIdx)*/
                /*    sp(xOff + endIdx, yOff + row) = 255.0f * endCovered;*/

                if (startIdx >= 0)
                    sp(xOff + startIdx, yOff + row) = utils::clamp(255.0f * startCovered, 0.0f, 255.0f);
                if (startIdx != endIdx)
                    sp(xOff + endIdx, yOff + row) = utils::clamp(255.0f * endCovered, 0.0f, 255.0f);

                for (int col = startIdx + 1; col < endIdx; ++col)
                    if (col >= 0)
                        sp(xOff + col, yOff + row) = 255;
            }
        }
    }
}

void
Rasterizer::rasterizeAscii(IAllocator* pAlloc, Font* pFont, f32 scale)
{
    if (!pAlloc)
    {
        LOG_WARN("pAlloc: {}\n", pFont);
        return;
    }

    if (!pFont)
    {
        LOG_WARN("pFont: {}\n", pFont);
        return;
    }

    const int iScale = std::round(scale);
    m_scale = scale;

    ssize nSquares = 10;
    ssize size = math::sq(scale) * math::sq(nSquares);

    m_altas.m_eType = Image::TYPE::MONO;
    m_altas.m_uData.pMono = pAlloc->zallocV<u8>(size);
    m_altas.m_width = nSquares * scale;
    m_altas.m_height = nSquares * scale;

    Arena arena(SIZE_8M);
    defer( arena.freeAll() );

    auto sp = m_altas.spanMono();

    i16 xOff = 0;
    i16 yOff = 0;
    const i16 xStep = iScale * X_STEP;

    for (u32 ch = '!'; ch <= '~'; ++ch)
    {
        m_mapCodeToXY.insert(pAlloc, ch, {xOff, yOff});

        Glyph g = pFont->readGlyph(ch);
        rasterizeGlyph(&arena, pFont, &g, xOff, yOff);

        if ((xOff += xStep) >= (nSquares * iScale) - xStep)
        {
            xOff = 0;
            yOff += iScale;
        }

        arena.reset();
    }
}

void
Rasterizer::destroy(adt::IAllocator* pAlloc)
{
    pAlloc->free(m_altas.m_uData.pMono);
    m_mapCodeToXY.destroy(pAlloc);
    *this = {};
}

} /* namespace ttf */
