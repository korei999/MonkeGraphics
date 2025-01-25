/* Rasterization goes here */

#include "draw.hh"
#include "adt/logs.hh"
#include "app.hh"
#include "frame.hh"

#include "adt/math.hh"

using namespace adt;

namespace draw
{

[[nodiscard]] inline math::V2
projectPoint(math::V3 pos)
{
    using namespace adt::math;
    auto& win = *app::g_pWindow;
    Span2D sp = win.getSurfaceBuffer();

    V2 res = pos.xy / pos.z;
    res = 0.5f * (res + V2{1.0f, 1.0f});
    res *= V2{static_cast<f32>(sp.getWidth()), static_cast<f32>(sp.getHeight())};

    return res;
}

static void
drawTriangle(Span<math::V3> spPoints, u32 color)
{
    using namespace adt::math;

    auto& win = *app::g_pWindow;
    Span2D sp = win.getSurfaceBuffer();

    V2 pointA = projectPoint(spPoints[0]);
    V2 pointB = projectPoint(spPoints[1]);
    V2 pointC = projectPoint(spPoints[2]);

    V2 edge0 = pointB - pointA;
    V2 edge1 = pointC - pointB;
    V2 edge2 = pointA - pointC;

    for (ssize y = 0; y < sp.getHeight(); ++y)
    {
        for (ssize x = 0; x < sp.getWidth(); ++x)
        {
            V2 pixPoint = V2{static_cast<f32>(x), static_cast<f32>(y)} + V2{0.5f, 0.5f};

            V2 pixEdge0 = pixPoint - pointA; 
            V2 pixEdge1 = pixPoint - pointB; 
            V2 pixEdge2 = pixPoint - pointC; 

            {
                /*LOG("{:.5}, {:.5}, {:.5}\n", math::V2Cross(pixEdge0, edge0), math::V2Cross(pixEdge1, edge1), math::V2Cross(pixEdge2, edge2));*/
            }

            /* inside triangle */
            if (math::V2Cross(pixEdge0, edge0) >= 0.0f &&
                math::V2Cross(pixEdge1, edge1) >= 0.0f &&
                math::V2Cross(pixEdge2, edge2) >= 0.0f
            )
            {
                sp(x, y).data = color;
            }
        }
    }
}

static void
gradientTest()
{
    auto& win = *app::g_pWindow;
    Span2D sp = win.getSurfaceBuffer();

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
    Span2D sp = win.getSurfaceBuffer();

    /* clear */
    utils::set(sp.data(), 0, sp.getWidth() * sp.getHeight());

    u32 aColors[] {
        0xffff0000,
        0xff00ff00,
        0xff0000ff,
    };

    for (ssize triangleIdx = 10; triangleIdx >= 0; --triangleIdx)
    {
        f32 depth = std::pow(2, triangleIdx + 1);

        V3 aPoints[] {
            {-1.0f, -0.5f, depth},
            {0.0f, 0.5f, depth},
            {1.0f, -0.5f, depth},
        };

        for (auto& point : aPoints)
        {
            point += V3{
                std::cosf(f::g_gt),
                std::sinf(f::g_gt), 0.0f
            };
        }

        drawTriangle(aPoints, aColors[triangleIdx % utils::size(aColors)]);
    }

}

} /* namespace draw */
