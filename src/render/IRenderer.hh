#pragma once

#include "adt/Arena.hh"

namespace render
{

struct IRenderer
{
    virtual void init() = 0;
    virtual void drawEntities(adt::Arena* pArena) = 0;
    virtual void destroy() = 0;
};

} /* namespace render */
