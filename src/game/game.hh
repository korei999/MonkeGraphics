#pragma once

#include "Entity.hh"

#include "adt/Arena.hh"
#include "adt/PoolSOA.hh"

namespace game
{

constexpr int MAX_ENTITIES = 256;

void loadStuff();
void updateState(adt::Arena* pArena);

extern adt::PoolSOA<Entity, EntityBind, MAX_ENTITIES,
    &Entity::pos, &Entity::rot, &Entity::scale,
    &Entity::vel,
    &Entity::assetI, &Entity::shaderI, &Entity::modelI,
    &Entity::bInvisible
> g_poolEntites;

} /* namespace game */
