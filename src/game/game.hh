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
[[nodiscard]] adt::isize searchEntity(adt::StringView svName);

using EntityVec = adt::VecSOAM<Entity, Entity::Bind,
    &Entity::sfName,
    &Entity::color,
    &Entity::pos, &Entity::rot, &Entity::scale,
    &Entity::vel,
    &Entity::assetI, &Entity::modelI,
    &Entity::eType,
    &Entity::bNoDraw
>;

extern EntityVec g_vEntities;

extern adt::MapManaged<adt::StringFixed<128>, adt::isize> g_mapNamesToEntities;

extern adt::isize g_dirLight;
extern adt::math::V3 g_ambientLight;

} /* namespace game */
