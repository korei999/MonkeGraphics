#pragma once

#include "adt/IAllocator.hh"
#include "adt/Span2D.hh"
#include "adt/types.hh"

enum IMAGE_TYPE : adt::u8
{
    RGBA, RGB
};

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
    union
    {
        ImagePixelRGBA* pRGBA;
        ImagePixelRGB* pRGB;
    } m_uData {};
    adt::i16 m_width {};
    adt::i16 m_height {};
    IMAGE_TYPE m_eType {};

    /* */

    [[nodiscard]] Image cloneToRGBA(adt::IAllocator* pAlloc);
    void swapRedBlue();
    void flipVertically(adt::IAllocator* pAlloc);

    adt::Span2D<ImagePixelRGBA>
    getSpanRGBA()
    {
        return {m_uData.pRGBA, m_width, m_height, m_width};
    }

    const adt::Span2D<ImagePixelRGBA>
    getSpanARGB() const
    {
        return {m_uData.pRGBA, m_width, m_height, m_width};
    }

    adt::Span2D<ImagePixelRGB>
    getSpanRGB()
    {
        return {m_uData.pRGB, m_width, m_height, m_width};
    }

    const adt::Span2D<ImagePixelRGB>
    getSpanRGB() const
    {
        return {m_uData.pRGB, m_width, m_height, m_width};
    }
};
