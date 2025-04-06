#pragma once

namespace adt
{
    struct Arena;
} /* namespace adt */

namespace render::gl::ui
{

void init();
void draw(adt::Arena* pArena);

} /* namespace render::gl::ui */
