/* Rasterization goes here */

#include "draw.hh"
#include "app.hh"
#include "frame.hh"

using namespace adt;

namespace draw
{

[[nodiscard]] inline math::V2
projectPoint(math::V3 pos)
{
    using namespace adt::math;
    const auto& win = *app::g_pWindow;

    V2 res = pos.xy / pos.z;
    res = 0.5f * (res + V2{1, 1}) * V2{static_cast<f32>(win.m_width), static_cast<f32>(win.m_height)};

    return res;
}

static void
gradientCrap(Span2D<Pixel> sp)
{
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
toBuffer(Span2D<Pixel> sp)
{
    using namespace adt::math;
    namespace f = frame;

    utils::set(sp.data(), 0, sp.getWidth() * sp.getHeight());

    for (ssize i = 0; i < 10; ++i)
    {
        f32 depth = std::pow(2, i + 1);

        V3 points[] {
            {-1.0f, -1.0f, depth},
            {1.0f, -1.0f, depth},
            {0.0f, 1.0f, depth},
        };

        for (auto& point : points)
        {
            V3 transformedPos = point + V3{std::cosf(f::g_gt*f::g_dt*4), std::sinf(f::g_gt*f::g_dt*4), 0.0f};
            V2 pixelPos = projectPoint(transformedPos);

            if (pixelPos.x >= 0 && pixelPos.x < sp.getWidth() &&
                pixelPos.y >= 0 && pixelPos.y < sp.getHeight()
            )
            {
                sp(pixelPos.x, pixelPos.y).data = 0xff00ff00;
            }
        }
    }
}

} /* namespace draw */
