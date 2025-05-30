#pragma once

#include "adt/QueueSPSC.hh"

#ifdef OPT_SW
    #include "adt/Span2D.hh"
    #include "adt/Vec.hh"
    #include "adt/simd.hh"

    #include "Image.hh"
    #include "colors.hh"
#endif

namespace adt
{

struct IAllocator;
struct StringView;

}

struct IWindow
{
    adt::IAllocator* m_pAlloc {};
    const char* m_ntsName {};

    int m_width {};
    int m_height {};
    int m_stride {};
    int m_realWidth {};
    int m_realHeight {};

    int m_winWidth {};
    int m_winHeight {};

    bool m_bRunning {};
    bool m_bPaused {};
    bool m_bPointerRelativeMode {};
    bool m_bHideCursor {};
    bool m_bFullscreen {};
    int m_swapInterval {};
    adt::f64 m_hideCursorTimeoutMS {};

    adt::f32 m_pointerSurfaceX {};
    adt::f32 m_pointerSurfaceY {};

    adt::f32 m_relMotionX {};
    adt::f32 m_relMotionY {};

    adt::QueueSPSC<adt::f32, 32> m_qWheelEvents {};


    /* */

    IWindow() = default;
    IWindow(adt::IAllocator* pAlloc, const char* ntsName)
        : m_pAlloc(pAlloc), m_ntsName(ntsName) {}

    /* */

    virtual void start(int width, int height) = 0;
    virtual void disableRelativeMode() = 0;
    virtual void enableRelativeMode() = 0;
    virtual void togglePointerRelativeMode() = 0;
    virtual void toggleFullscreen() = 0;
    virtual void hideCursor(bool bHide) = 0;
    virtual void setCursorImage(adt::StringView cursorType) = 0;
    virtual void setFullscreen() = 0;
    virtual void unsetFullscreen() = 0;
    virtual void setSwapInterval(int interval) = 0;
    virtual void toggleVSync() = 0;
    virtual void swapBuffers() = 0;
    virtual void procEvents() = 0;
    virtual void showWindow() = 0;
    virtual void destroy() = 0;
    virtual void bindContext() = 0;
    virtual void unbindContext() = 0;

#ifdef OPT_SW
    void (*m_pfnUpdateCB)(void*) {};
    void* m_pDrawArg {};
    adt::Vec<adt::f32> m_vDepthBuffer {};

    /* */

    virtual adt::Span2D<ImagePixelRGBA> surfaceBuffer() = 0;
    virtual void scheduleFrame() = 0;

    /* */

    adt::Span2D<adt::f32>
    depthBuffer()
    {
        return {
            m_vDepthBuffer.data(), m_width, m_height, m_stride
        };
    }

    void
    clearSurfaceBuffer(adt::math::V4 color)
    {
#ifdef ADT_AVX2
        adt::simd::i32Fillx8(
            adt::Span<adt::i32>{
                (adt::i32*)surfaceBuffer().data(),
                surfaceBuffer().getStride() * surfaceBuffer().getHeight()
            },
            colors::V4ToRGBA(color)
        );
#else

        adt::simd::i32Fillx4(
            adt::Span<adt::i32>{
                (adt::i32*)surfaceBuffer().data(),
                surfaceBuffer().getStride() * surfaceBuffer().getHeight()
            },
            colors::V4ToRGBA(color)
        );
#endif /* ADT_AVX2 */
    }

    void
    clearDepthBuffer()
    {
#ifdef ADT_AVX2
        adt::simd::f32Fillx8(
            {m_vDepthBuffer.data(), m_vDepthBuffer.size()},
            std::numeric_limits<adt::f32>::max()
        );
#else
        adt::simd::f32Fillx4(
            {m_vDepthBuffer.data(), m_vDepthBuffer.size()},
            std::numeric_limits<adt::f32>::max()
        );
#endif /* ADT_AVX2 */
    }

    void
    regUpdateCB(void (*pfn)(void*), void* pArg)
    {
        m_pfnUpdateCB = pfn;
        m_pDrawArg = pArg;
    }

    void
    update()
    {
        ADT_ASSERT(m_pfnUpdateCB, "no regUpdateCB()");
        m_pfnUpdateCB(m_pDrawArg);
        scheduleFrame();
    }

#endif
};
