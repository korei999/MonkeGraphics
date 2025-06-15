#include "common.hh"

using namespace adt;

namespace common
{

const Span2D<const ImagePixelRGBA> g_spDefaultTexture = [] {
    auto t = createDefaultTexture();
    return Span2D<const ImagePixelRGBA> {t.m_pData, t.width(), t.height(), t.stride()};
}();

Span2D<ImagePixelRGBA>
createDefaultTexture()
{
    constexpr int width = 8;
    constexpr int height = 8;

    static ImagePixelRGBA aPixels[width * height] {};

    Span2D sp(aPixels, width, height, width);
    for (int y = 0; y < sp.height(); ++y)
    {
        for (int x = 0; x < sp.width(); ++x)
        {
            u32 colorChannel = 255 * ((x + (y % 2)) % 2);
            ImagePixelRGBA p;
            p.r = 128;
            p.g = static_cast<u8>(colorChannel);
            p.b = 0;
            p.a = 255;

            sp(x, y) = p;
        }
    }

    return sp;
}

bool
AABB(
    const adt::f32 px,
    const adt::f32 py,
    const adt::f32 xOff,
    const adt::f32 yOff,
    const adt::f32 width,
    const adt::f32 height
)
{
    return
        py >= yOff &&
        py < yOff + height &&
        px >= xOff &&
        px < xOff + width;
}

} /* namespace common */
