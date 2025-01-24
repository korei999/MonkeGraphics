#pragma once

#include "IWindow.hh"

#include "wayland-protocols/xdg-shell.h"
#include "wayland-protocols/viewporter.h"

namespace platform::wayland
{

struct Client final : public IWindow
{
    wl_display* m_pDisplay {};
    wl_registry* m_pRegistry {};
    wl_compositor* m_pCompositor {};
    wl_surface* m_pSurface {};
    wl_output* m_pOutput {};

    wl_shm* m_pShm {};
    wl_shm_pool* m_pShmPool {};
    adt::u8* m_pPoolData {};
    wl_buffer* m_pBuffer {};

    wl_seat* m_pSeat {};
    wl_keyboard* m_pKeyboard {};
    wl_pointer* m_pPointer {};

    xdg_wm_base* m_pXdgWmBase {};
    xdg_surface* m_pXdgSurface {};
    xdg_toplevel* m_pXdgToplevel {};
    bool m_bConfigured {};
    int m_newWidth {};
    int m_newHeight {};

    wl_callback* m_pCallBack {};

    wp_viewporter* m_pViewporter {};
    wp_viewport* m_pViewport {};

    /* */

    Client() = default;
    Client(const char* sName) : IWindow(sName) {}

    /* */

    virtual void start() override;
    virtual adt::Span2D<draw::Pixel> getSurfaceBuffer() override;
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
    virtual void scheduleFrame() override;

    /* */

    void global(wl_registry* pRegistry, uint32_t name, const char* ntsInterface, uint32_t version);
    void globalRemove(wl_registry* pRegistry, uint32_t name);

    void shmFormat(wl_shm* pShm, uint32_t format);

    void xdgWmBasePing(xdg_wm_base* pXdgWmBase, uint32_t serial);

    void xdgSurfaceConfigure(struct xdg_surface* pXdgSurface, uint32_t serial);

    void xdgToplevelConfigure(xdg_toplevel* pXdgToplevel, int32_t width, int32_t height, wl_array* pStates);
    void xdgToplevelClose(struct xdg_toplevel* pToplevel);
    void xdgToplevelConfigureBounds(xdg_toplevel* pToplevel, int32_t width, int32_t height);
    void xdgToplevelWmCapabilities(xdg_toplevel* pToplevel, wl_array* pCapabilities);

    void seatCapabilities(wl_seat* pWlSeat, uint32_t capabilities);
    void seatName(wl_seat* pWlSeat, const char* ntsName);

    void outputGeometry(
        wl_output* pOutput, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight,
        int32_t subpixel, const char* ntsMake, const char* ntsModel, int32_t transform
    );
    void outputMode(wl_output* pOutput, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
    void outputDone(wl_output* pOutput);
    void outputScale(wl_output* pOutput, int32_t factor);
    void outputName(wl_output* pOutput, const char* ntsName);
    void outputDescription(wl_output* pOutput, const char* ntsDescription);

    /* input */
    void keyboardKeymap(wl_keyboard* pKeyboard, uint32_t format, int32_t fd, uint32_t size);
    void keyboardEnter(wl_keyboard* pKeyboard, uint32_t serial, wl_surface* pSurface, wl_array* pKeys);
    void keyboardLeave(wl_keyboard* pKeyboard, uint32_t serial, wl_surface* pSurface);
    void keyboardKey(wl_keyboard* pKeyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void keyboardModifiers(
        wl_keyboard* pKeyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched,
        uint32_t modsLocked, uint32_t group
    );
    void keyboardRepeatInfo(wl_keyboard* pKeyboard, int32_t rate, int32_t delay);
    /* */

    void callbackDone(wl_callback* pCallback, uint32_t callbackData);
};

} /* namespace platform::wayland */
