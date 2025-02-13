#pragma once

#include "../IRenderer.hh"

namespace render::sw
{

struct Renderer : public IRenderer
{
    virtual void init() override;
    virtual void drawEntities(adt::Arena* pArena) override;
};

} /* namespace render::sw */
