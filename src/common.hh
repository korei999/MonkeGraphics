#pragma once

#include "Image.hh"

namespace common
{

adt::Span2D<ImagePixelRGBA> createDefaultTexture();

extern const adt::Span2D<ImagePixelRGBA> g_spDefaultTexture;

} /* namespace common */
