#pragma once

#include "ttf/types.hh"

#include "Image.hh"

#include "adt/Pair.hh"

namespace ttf
{

struct Font;

struct Rasterizer
{
    static constexpr adt::f32 X_STEP = 0.60f; /* x-axis spacing between glyphs */

    /* */

    Image m_altas {};
    adt::Map<adt::u32, adt::Pair<adt::i16, adt::i16>, adt::hash::dumbFunc> m_mapCodeToXY {};
    adt::f32 m_scale {};

    /* */

    void destroy(adt::IAllocator* pAlloc);
    void rasterizeAscii(adt::IAllocator* pAlloc, Font* pFont, adt::f32 scale);

protected:
    void rasterizeGlyph(const Font& pFont, const Glyph& pGlyph, int xOff, int yOff);
};

} /* namespace ttf */
