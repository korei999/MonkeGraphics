#pragma once

#include "Image.hh"

namespace common
{

adt::Span2D<ImagePixelRGBA> createDefaultTexture();

extern const adt::Span2D<const ImagePixelRGBA> g_spDefaultTexture;

bool AABB(
    const adt::f32 px,
    const adt::f32 py,
    const adt::f32 xOff,
    const adt::f32 yOff,
    const adt::f32 width,
    const adt::f32 height
);

} /* namespace common */
