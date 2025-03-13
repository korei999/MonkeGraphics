#pragma once

#include "adt/Arena.hh"

namespace render
{

struct IRenderer
{
    virtual void init() = 0;
    virtual void drawGame(adt::Arena* pArena) = 0;
    virtual void destroy() = 0;
};

} /* namespace render */
