#pragma once

#include "Image.hh"
#include "gltf/types.hh"

namespace common
{

// void updateModelNode(animation::Animator* pAnimator, const gltf::Node& node, adt::math::M4* pTrm);
adt::Span2D<ImagePixelRGBA> createDefaultTexture();

extern const adt::Span2D<ImagePixelRGBA> g_spDefaultTexture;

} /* namespace common */
