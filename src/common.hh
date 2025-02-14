#pragma once

#include "Image.hh"
#include "gltf/gltf.hh"

namespace common
{

void updateModelNode(gltf::Model* pModel, gltf::Node* pNode, adt::math::M4* pTrm);
adt::Span2D<ImagePixelRGBA> createDefaultTexture();

extern const adt::Span2D<ImagePixelRGBA> g_spDefaultTexture;

} /* namespace common */
