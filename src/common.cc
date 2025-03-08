#include "common.hh"

using namespace adt;

namespace common
{

const Span2D<ImagePixelRGBA> g_spDefaultTexture = createDefaultTexture();

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

} /* namespace common */
