#pragma once

#include "Entity.hh"

#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/VecSOA.hh"

namespace game
{

constexpr int MAX_ENTITIES = 256;

void loadStuff();
void updateState(adt::Arena* pArena);
/* NPOS */ [[nodiscard]] adt::isize searchEntity(adt::StringView svName);

using VSOAEntity = adt::VecSOAM<ENTITY_TEMPLATE_ARGS>;

extern VSOAEntity g_vEntities;

extern adt::MapM<adt::StringFixed<128>, adt::isize> g_mapNamesToEntities;

extern adt::isize g_dirLight;
extern adt::math::V3 g_ambientLight;

} /* namespace game */
