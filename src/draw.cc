/* Rasterization goes here */

#include "draw.hh"
#include "app.hh"
#include "colors.hh"
#include "frame.hh"

#include "adt/math.hh"

using namespace adt;

namespace draw
{

struct Vertex
{
    math::V3 pos {};
};

[[nodiscard]] inline math::V2
projectPoint(math::V3 pos)
{
    using namespace adt::math;
    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();

    V2 res = pos.xy / pos.z;
    res = 0.5f * (res + V2{1.0f, 1.0f});
    res *= V2{static_cast<f32>(sp.getWidth()), static_cast<f32>(sp.getHeight())};

    return res;
}

static void
drawTriangle(Span<math::V3> spPoints, math::V3* pColors, const math::M4& trm)
{
    using namespace adt::math;

    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();
    Span2D spDepth = win.depthBuffer();

    V3 trPoint0 = (trm * V4From(spPoints[0], 1.0f)).xyz;
    V3 trPoint1 = (trm * V4From(spPoints[1], 1.0f)).xyz;
    V3 trPoint2 = (trm * V4From(spPoints[2], 1.0f)).xyz;

    V2 pointA = projectPoint(trPoint0);
    V2 pointB = projectPoint(trPoint1);
    V2 pointC = projectPoint(trPoint2);

    int minX = utils::min(utils::min(static_cast<int>(pointA.x),             static_cast<int>(pointB.x)),             static_cast<int>(pointC.x));
    int maxX = utils::max(utils::max(static_cast<int>(std::round(pointA.x)), static_cast<int>(std::round(pointB.x))), static_cast<int>(std::round(pointC.x)));
    int minY = utils::min(utils::min(static_cast<int>(pointA.y),             static_cast<int>(pointB.y)),             static_cast<int>(pointC.y));
    int maxY = utils::max(utils::max(static_cast<int>(std::round(pointA.y)), static_cast<int>(std::round(pointB.y))), static_cast<int>(std::round(pointC.y)));

    minX = utils::clamp(minX, 0, static_cast<int>(sp.getWidth() - 1));
    maxX = utils::clamp(maxX, 0, static_cast<int>(sp.getWidth() - 1));
    minY = utils::clamp(minY, 0, static_cast<int>(sp.getHeight() - 1));
    maxY = utils::clamp(maxY, 0, static_cast<int>(sp.getHeight() - 1));

    V2 edge0 = pointB - pointA;
    V2 edge1 = pointC - pointB;
    V2 edge2 = pointA - pointC;

    bool bTopLeft0 = (edge0.y > 0.0f) || (edge0.x > 0.0f && edge0.y == 0.0f);
    bool bTopLeft1 = (edge1.y > 0.0f) || (edge1.x > 0.0f && edge1.y == 0.0f);
    bool bTopLeft2 = (edge2.y > 0.0f) || (edge2.x > 0.0f && edge2.y == 0.0f);

    f32 barycentricDiv = V2Cross(pointB - pointA, pointC - pointA);

    for (ssize y = minY; y <= maxY; ++y)
    {
        for (ssize x = minX; x <= maxX; ++x)
        {
            V2 pixPoint = V2{static_cast<f32>(x), static_cast<f32>(y)} + V2{0.5f, 0.5f};

            V2 pixEdge0 = pixPoint - pointA; 
            V2 pixEdge1 = pixPoint - pointB; 
            V2 pixEdge2 = pixPoint - pointC; 

            f32 crossLen0 = V2Cross(pixEdge0, edge0);
            f32 crossLen1 = V2Cross(pixEdge1, edge1);
            f32 crossLen2 = V2Cross(pixEdge2, edge2);

            /* inside triangle */
            if ((crossLen0 > 0.0f || (bTopLeft0 && crossLen0 == 0.0f)) &&
                (crossLen1 > 0.0f || (bTopLeft1 && crossLen1 == 0.0f)) &&
                (crossLen2 > 0.0f || (bTopLeft2 && crossLen2 == 0.0f))
            )
            {
                ssize invY = sp.getHeight() - 1 - y;

                f32 t0 = -crossLen1 / barycentricDiv;
                f32 t1 = -crossLen2 / barycentricDiv;
                f32 t2 = -crossLen0 / barycentricDiv;

                f32 depth = 1.0f / (t0 * (1.0f / trPoint0.z) + t1 * (1.0f / trPoint1.z) + t2 * (1.0f / trPoint2.z));
                if (depth < spDepth(x, invY))
                {
                    V3 finalCol = (t0 * pColors[0] + t1 * pColors[1] + t2 * pColors[2]);
                    V4 cv4; cv4.xyz = finalCol; cv4.w = 255.0f;

                    sp(x, invY).data = colors::V4ToARGB(cv4);;
                    spDepth(x, invY) = depth;
                }
            }
        }
    }
}

[[maybe_unused]] static void
gradientTest()
{
    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();

    static int frame = 0;

    for (ssize y = 0; y < sp.getHeight(); ++y)
    {
        for (ssize x = 0; x < sp.getWidth(); ++x)
        {
            Pixel pix {
                .b = static_cast<u8>(x),
                .g = static_cast<u8>(x + frame),
                .r = static_cast<u8>(y - frame),
                .a = 255,
            };

            sp(x, y).data = pix.data;
        }
    }

    ++frame;
}

void
toBuffer()
{
    using namespace adt::math;
    namespace f = frame;

    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();

    /* clear */
    win.clearBuffer();
    win.clearDepthBuffer();

    V3 aPositions0[] {
        { 0.0f,  0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f},
    };

    V3 aPositions1[] {
        { 0.0f,  0.5f, 0.0f},
        { 0.5f, -0.5f, 0.2f},
        {-0.5f, -0.5f, -0.2f},
    };

    V3 aPositions2[] {
        { 0.0f, -0.5f, -0.4f},
        {-1.5f,  0.5f, 2.0f},
        { 1.5f,  0.5f, 2.0f},
    };

    V3 aColors0[] {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };

    V3 aColors1[] {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
    };

    // auto off = std::sinf(f::g_time * 0.002) * 0.5f + 1.0f;
    // M4 tr = M4GetTranslation({0, 0, 1.0f + off})
    //     * M4GetScale(off + 0.2f, 1, off + 0.2f);

    M4 tr = M4TranslationFrom({0, 0, 1}) * M4RotZ(M4Iden(), f::g_time * 0.002);

    drawTriangle(aPositions1, aColors1, tr);
    drawTriangle(aPositions0, aColors0, tr);
    drawTriangle(aPositions2, aColors1, tr);
}

} /* namespace draw */
