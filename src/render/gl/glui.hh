#pragma once

namespace adt
{
    struct ArenaList;
} /* namespace adt */

namespace render::gl::ui
{

void init();
void draw(adt::ArenaList* pArena);

} /* namespace render::gl::ui */
