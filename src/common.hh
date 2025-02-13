#pragma once

#include "gltf/gltf.hh"

namespace common
{

void updateModelNode(gltf::Model* pModel, gltf::Node* pNode, adt::math::M4* pTrm);

} /* namespace common */
