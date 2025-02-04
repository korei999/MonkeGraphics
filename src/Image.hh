#pragma once

#include "adt/IAllocator.hh"
#include "adt/Span2D.hh"
#include "adt/types.hh"

enum IMAGE_TYPE : adt::u8
{
    ARGB, RGB
};

union ImagePixelARGB
{
    struct { adt::u8 b, g, r, a; };
    adt::u32 data;
    adt::i32 iData;
};

union ImagePixelRGB
{
    struct { adt::u8 b, g, r; };
    adt::u8 aData[3];
};

struct Image
{
    union
    {
        ImagePixelARGB* pARGB;
        ImagePixelRGB* pRGB;
    } m_uData {};
    adt::i16 m_width {};
    adt::i16 m_height {};
    IMAGE_TYPE m_eType {};

    /* */

    [[nodiscard]] Image cloneToARGB(adt::IAllocator* pAlloc);

    adt::Span2D<ImagePixelARGB>
    getSpanARGB()
    {
        return {m_uData.pARGB, m_width, m_height, m_width};
    }

    const adt::Span2D<ImagePixelARGB>
    getSpanARGB() const
    {
        return {m_uData.pARGB, m_width, m_height, m_width};
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
