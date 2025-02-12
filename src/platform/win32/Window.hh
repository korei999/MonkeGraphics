#pragma once

#include "IWindow.hh"

namespace platform::win32
{

struct Window : public IWindow
{
    HINSTANCE m_hInstance;
    HWND m_hWindow;
    HDC m_hDeviceContext;
    HGLRC m_hGlContext;
    WNDCLASSEXW m_windowClass;
    RAWINPUTDEVICE m_rawInputDevices[2];

    adt::VecBase<ImagePixelRGBA> m_vSurfaceBuffer {};

    /* */

    Window() = default;
    Window(adt::IAllocator* pAlloc, const char* ntsName);

    /* */

    virtual void start(int width, int height) override;
    virtual void disableRelativeMode() override;
    virtual void enableRelativeMode() override;
    virtual void togglePointerRelativeMode() override;
    virtual void toggleFullscreen() override;
    virtual void hideCursor(bool bHide) override;
    virtual void setCursorImage(adt::String cursorType) override;
    virtual void setFullscreen() override;
    virtual void unsetFullscreen() override;
    virtual void setSwapInterval(int interval) override;
    virtual void toggleVSync() override;
    virtual void swapBuffers() override;
    virtual void procEvents() override;
    virtual void showWindow() override;
    virtual void destroy() override;
    virtual void bindContext() override;
    virtual void unbindContext() override;
    virtual adt::Span2D<ImagePixelRGBA> surfaceBuffer() override;
    virtual void scheduleFrame() override;

    /* */

    void procKey(WPARAM keyCode, bool bDown);
    void registerRawMouseDevice(bool bOn);
    static bool enterFullscreen(HWND hwnd, int fullscreenWidth, int fullscreenHeight, int colorBits, int refreshRate);
    static bool exitFullscreen(HWND hwnd, int windowX, int windowY, int windowedWidth, int windowedHeight, int windowedPaddingX, int windowedPaddingY);
};

} /* namespace platform::win32 */
