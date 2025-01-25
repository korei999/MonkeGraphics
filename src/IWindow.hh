#pragma once

#include "adt/String.hh"
#include "adt/Span2D.hh"

#include "draw.hh"

/* Platform abstracted application/window interface */
struct IWindow
{
    adt::IAllocator* m_pAlloc {};
    const char* m_ntsName {};

    int m_width {};
    int m_height {};
    int m_winWidth {};
    int m_winHeight {};

    bool m_bRunning {};
    bool m_bPaused {};
    bool m_bPointerRelativeMode {};
    bool m_bHideCursor {};
    bool m_bFullscreen {};
    int m_swapInterval {};
    adt::f64 m_hideCursorTimeoutMS {};

    void (*m_pfnDrawCB)(void*) {};
    void* m_pDrawArg {};

    /* */

    IWindow() = default;
    IWindow(adt::IAllocator* pAlloc, const char* ntsName)
        : m_pAlloc(pAlloc), m_ntsName(ntsName) {}

    /* */

    virtual void start() = 0;
    virtual adt::Span2D<draw::Pixel> getSurfaceBuffer() = 0;
    virtual void disableRelativeMode() = 0;
    virtual void enableRelativeMode() = 0;
    virtual void togglePointerRelativeMode() = 0;
    virtual void toggleFullscreen() = 0;
    virtual void hideCursor(bool bHide) = 0;
    virtual void setCursorImage(adt::String cursorType) = 0;
    virtual void setFullscreen() = 0;
    virtual void unsetFullscreen() = 0;
    virtual void setSwapInterval(int interval) = 0;
    virtual void toggleVSync() = 0;
    virtual void swapBuffers() = 0;
    virtual void procEvents() = 0;
    virtual void showWindow() = 0;
    virtual void destroy() = 0;
    virtual void scheduleFrame() = 0;

    /* */

    void
    regDrawCB(void (*pfn)(void*), void* pArg)
    {
        m_pfnDrawCB = pfn;
        m_pDrawArg = pArg;
    }

    void
    draw()
    {
        ADT_ASSERT(m_pfnDrawCB, " ");
        m_pfnDrawCB(m_pDrawArg);
        scheduleFrame();
    }
};
