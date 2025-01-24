/* Rasterization goes here */

#include "draw.hh"

using namespace adt;

namespace draw
{

void
toBuffer(Span2D<Pixel> sp)
{
    static int frame = 0;

    for (ssize y = 0; y < sp.getHeight(); ++y)
    {
        for (ssize x = 0; x < sp.getWidth(); ++x)
        {
            Pixel pix {
                .b = 64,
                .g = static_cast<u8>((x + frame) % 256),
                .r = static_cast<u8>((y + frame) % 256),
                .a = 255,
            };

            sp(x, y).data = pix.data;
        }
    }

    ++frame;
}

} /* namespace draw */
