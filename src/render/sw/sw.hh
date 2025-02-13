#pragma once

#include "../IRenderer.hh"

namespace render::sw
{

struct Renderer : public IRenderer
{
    virtual void init() override;
    virtual void drawEntities(adt::Arena* pArena) override;
};

void drawEntities(adt::Arena* pAlloc);

} /* namespace render::sw */
