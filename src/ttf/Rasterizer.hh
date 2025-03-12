#pragma once

#include "ttf/types.hh"

#include "Image.hh"

namespace ttf
{

struct Font;

struct Rasterizer
{
    Image m_altas {};
    adt::Map<adt::u32, adt::Pair<adt::i16, adt::i16>> m_mapCodeToXY {};
    adt::f32 m_scale {};

    /* */

    void rasterizeGlyph(adt::IAllocator* pAlloc, Font* pFont, Glyph* pGlyph, adt::Span2D<adt::u8> spBitmap);
    void rasterizeAscii(adt::IAllocator* pAlloc, Font* pFont, adt::f32 scale);
    void destroy(adt::IAllocator* pAlloc);
};

} /* namespace ttf */
