#pragma once

#include "types.hh"

#include "adt/bin.hh"

namespace ttf
{

struct Font
{
    adt::IAllocator* m_pAlloc {};
    adt::String m_sFile {};
    adt::bin::Reader m_bin {};
    TableDirectory m_tableDirectory {};
    Head m_head {};
    Cmap m_cmap {};
    CmapFormat4 m_cmapF4 {};
    adt::Map<adt::u32, Glyph> m_mapOffsetToGlyph {};

    /* */

    Font() = default;
    Font(adt::IAllocator* pAlloc, adt::StringView svFile)
        : m_pAlloc(pAlloc), m_sFile(adt::String(pAlloc, svFile)), m_bin(m_sFile) {}

    /* */

    bool parse();
    Glyph* readGlyph(adt::u32 codePoint);
    void printGlyphDBG(const Glyph& g, bool bNormalize = false);
    void destroy();

    /* */

private:
    bool parse2();
    adt::MapResult<adt::StringView, TableRecord> getTable(adt::StringView sTableTag);
    void readHeadTable();
    void readCmapTable();
    void readCmap(adt::u32 offset);
    void readCmapFormat4();
    adt::u32 getGlyphOffset(adt::u32 idx);
    adt::u32 getGlyphIdx(adt::u16 code);
    FWord readFWord();
    void readCompoundGlyph(Glyph* g);
    void readSimpleGlyph(Glyph* g);
};

} /* namespace ttf */
