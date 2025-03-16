#pragma once

#include "Entity.hh"

#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/PoolSOA.hh"

namespace game
{

constexpr int MAX_ENTITIES = 256;

void loadStuff();
void updateState(adt::Arena* pArena);
[[nodiscard]] adt::PoolSOAHandle<Entity> searchEntity(adt::StringView svName);

extern adt::PoolSOA<Entity, Entity::Bind, MAX_ENTITIES,
    &Entity::sfName,
    &Entity::color,
    &Entity::pos, &Entity::rot, &Entity::scale,
    &Entity::vel,
    &Entity::assetI, &Entity::modelI,
    &Entity::eType,
    &Entity::bNoDraw
> g_poolEntities;

extern adt::MapManaged<
    adt::StringFixed<128>,
    adt::PoolSOAHandle<Entity>
> g_mapNamesToEntities;

extern adt::PoolSOAHandle<Entity> g_dirLight;

} /* namespace game */
