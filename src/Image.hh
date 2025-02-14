#pragma once

#include "adt/IAllocator.hh"
#include "adt/Span2D.hh"
#include "adt/types.hh"

union ImagePixelRGBA
{
    struct { adt::u8 r, g, b, a; };
    adt::u32 data;
    adt::i32 iData;
};

union ImagePixelRGB
{
    struct { adt::u8 r, g, b; };
    adt::u8 aData[3];
};

struct Image
{
    enum class TYPE : adt::u8 { RGBA, RGB };

    /* */

    union
    {
        ImagePixelRGBA* pRGBA;
        ImagePixelRGB* pRGB;
    } m_uData {};
    adt::i16 m_width {};
    adt::i16 m_height {};
    TYPE m_eType {};

    /* */

    [[nodiscard]] Image cloneToRGBA(adt::IAllocator* pAlloc);
    void swapRedBlue();
    void flipVertically(adt::IAllocator* pAlloc);

    adt::Span2D<ImagePixelRGBA>
    getSpanRGBA()
    {
        ADT_ASSERT(m_eType == TYPE::RGBA, " ");
        return {m_uData.pRGBA, m_width, m_height, m_width};
    }

    const adt::Span2D<ImagePixelRGBA>
    getSpanARGB() const
    {
        ADT_ASSERT(m_eType == TYPE::RGBA, " ");
        return {m_uData.pRGBA, m_width, m_height, m_width};
    }

    adt::Span2D<ImagePixelRGB>
    getSpanRGB()
    {
        ADT_ASSERT(m_eType == TYPE::RGB, " ");
        return {m_uData.pRGB, m_width, m_height, m_width};
    }

    const adt::Span2D<ImagePixelRGB>
    getSpanRGB() const
    {
        ADT_ASSERT(m_eType == TYPE::RGB, " ");
        return {m_uData.pRGB, m_width, m_height, m_width};
    }
};
