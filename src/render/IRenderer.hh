#pragma once

namespace adt
{

struct Arena;

}

namespace render
{

struct IRenderer
{
    virtual void init() = 0;
    virtual void drawEntities(adt::Arena* pArena) = 0;
};

} /* namespace render */
