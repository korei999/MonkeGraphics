#pragma once

#include "adt/types.hh"

namespace draw
{

union Pixel
{
    struct { adt::u8 b, g, r, a; };
    adt::u32 data;
};

void toBuffer();

} /* namespace draw */
