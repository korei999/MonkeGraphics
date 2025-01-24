#pragma once

#include "adt/Span2D.hh"
#include "adt/math.hh"

namespace draw
{

union Pixel
{
    struct { adt::u8 b, g, r, a; };
    adt::u32 data;
};

void toBuffer(adt::Span2D<Pixel> sp);

} /* namespace draw */
