#pragma once

#include "Entity.hh"

#include "adt/Arena.hh"

namespace game
{

void loadStuff();
void updateState(adt::Arena* pArena);

extern EntityPoolSOA<128> g_poolEntites;

} /* namespace game */
