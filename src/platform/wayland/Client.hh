#pragma once

#include "IWindow.hh"

#include "wayland-protocols/xdg-shell.h"
#include "wayland-protocols/viewporter.h"
#include "wayland-protocols/relative-pointer-unstable-v1.h"
#include "wayland-protocols/pointer-constraints-unstable-v1.h"
#include "wayland-protocols/xdg-decoration.h"

#include "adt/Vec.hh"

#include <wayland-cursor.h>

namespace platform::wayland
{

struct Client : public IWindow
{
    wl_display* m_pDisplay {};
    wl_registry* m_pRegistry {};
    wl_compositor* m_pCompositor {};
    wl_surface* m_pSurface {};

    adt::Vec<wl_output*> m_vOutputs {};

    wl_shm* m_pShm {};
    wl_shm_pool* m_pShmPool {};
    adt::u8* m_pPoolData {};
    adt::isize m_poolSize {};
    wl_buffer* m_pBuffer {};

    wl_seat* m_pSeat {};
    wl_keyboard* m_pKeyboard {};
    wl_pointer* m_pPointer {};
    adt::u32 m_lastPointerEnterSerial {};

    wl_surface* m_pPointerSurface {};
    wl_cursor_theme* m_pCursorTheme {};

    zwp_relative_pointer_manager_v1* m_pRelPointerMgr {};
    zwp_relative_pointer_v1* m_pRelPointer {};

    zwp_pointer_constraints_v1* m_pPointerConstraints {};
    zwp_locked_pointer_v1* m_pLockedPointer {};
    bool m_bRestoreLockedPointer {};

    xdg_wm_base* m_pXdgWmBase {};
    xdg_surface* m_pXdgSurface {};
    xdg_toplevel* m_pXdgToplevel {};
    bool m_bConfigured {};
    int m_newWidth {};
    int m_newHeight {};

    wp_viewporter* m_pViewporter {};
    wp_viewport* m_pViewport {};

    zxdg_decoration_manager_v1* m_pXdgDecorationManager {};
    zxdg_toplevel_decoration_v1* m_pXdgDecoration {};

    bool m_bKeyboardUnfocused {};
    bool m_bPointerUnfocused {};

    /* */

    Client() = default;
    Client(adt::IAllocator* pAlloc, const char* ntsName);

    /* */

    virtual void start(int width, int height) override;
    virtual void disableRelativeMode() override;
    virtual void enableRelativeMode() override;
    virtual void togglePointerRelativeMode() override;
    virtual void toggleFullscreen() override;
    virtual void hideCursor(bool bHide) override;
    virtual void setCursorImage(adt::StringView cursorType) override;
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

    /* */

#ifdef OPT_SW
    wl_callback* m_pCallBack {};
    adt::u8* m_pSurfaceBufferBind {};
    adt::Vec<ImagePixelRGBA> m_vTempBuff {};

    virtual adt::Span2D<ImagePixelRGBA> surfaceBuffer() override;
    virtual void scheduleFrame() override;

    /* */

    void updateSurface();
#endif

    /* */

    virtual void resizeCB(int width, int height);

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

    void outputGeometry(wl_output* pOutput, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight, int32_t subpixel, const char* ntsMake, const char* ntsModel, int32_t transform);
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
    void keyboardModifiers(wl_keyboard* pKeyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
    void keyboardRepeatInfo(wl_keyboard* pKeyboard, int32_t rate, int32_t delay);

    void pointerEnter(wl_pointer* pPointer, uint32_t serial, wl_surface* pSurface, wl_fixed_t surfaceX, wl_fixed_t surfaceY);
    void pointerLeave(wl_pointer* pPointer, uint32_t serial, wl_surface* pSurface);
    void pointerMotion(wl_pointer* pPointer, uint32_t time, wl_fixed_t surfaceX, wl_fixed_t surfaceY);
    void pointerButton(wl_pointer* pPointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    void pointerAxis(wl_pointer* pPointer, uint32_t time, uint32_t axis, wl_fixed_t value);
    void pointerFrame(wl_pointer* pPointer);
    void pointerAxisSource(wl_pointer* pPointer, uint32_t axisSource);
    void pointerAxisStop(wl_pointer* pPointer, uint32_t time, uint32_t axis);
    void pointerAxisDiscrete(wl_pointer* pPointer, uint32_t axis, int32_t discrete);
    void pointerAxisValue120(wl_pointer* pPointer, uint32_t axis, int32_t value120);
    void pointerAxisRelativeDirection(wl_pointer* pPointer, uint32_t axis, uint32_t direction);

    void relativePointerMotion(zwp_relative_pointer_v1* pRelPointerV1, uint32_t utimeHi, uint32_t utimeLo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dxUnaccel, wl_fixed_t dyUnaccel);

    void decorationConfigure(zxdg_toplevel_decoration_v1* pZXdgToplevelDecoration_v1, uint32_t mode);
    /* */

    void callbackDone(wl_callback* pCallback, uint32_t callbackData);
    void initShm();
};

} /* namespace platform::wayland */
