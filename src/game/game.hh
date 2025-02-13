#pragma once

#include "Entity.hh"

#include "adt/Arena.hh"
#include "gltf/gltf.hh"

namespace game
{

void loadStuff();
void updateState(adt::Arena* pArena);
void updateModelNode(gltf::Model* pModel, gltf::Node* pNode, adt::math::M4* pTrm);

extern EntityPoolSOA<128> g_aEntites;

} /* namespace game */
