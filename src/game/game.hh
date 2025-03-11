#pragma once

#include "Entity.hh"

#include "adt/Arena.hh"
#include "adt/Map.hh"
#include "adt/PoolSOA.hh"
#include "adt/Opt.hh"

namespace game
{

constexpr int MAX_ENTITIES = 256;

void loadStuff();
void updateState(adt::Arena* pArena);
[[nodiscard]] adt::Opt<adt::PoolSOAHandle<Entity>> searchEntity(adt::StringView svName);

extern adt::PoolSOA<Entity, EntityBind, MAX_ENTITIES,
    &Entity::sfName,
    &Entity::pos, &Entity::rot, &Entity::scale,
    &Entity::vel,
    &Entity::assetI, &Entity::modelI,
    &Entity::bNoDraw
> g_poolEntities;

extern adt::MapManaged<
    adt::StringFixed<128>,
    adt::PoolSOAHandle<Entity>
> g_mapNamesToEntities;

} /* namespace game */
