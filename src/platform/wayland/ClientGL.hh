#pragma once

#include "Client.hh"

#include <wayland-egl.h>
#include <EGL/egl.h>

namespace platform::wayland
{

struct ClientGL : public Client
{
    wl_egl_window* m_eglWindow {};
    EGLDisplay m_eglDisplay {};
    EGLContext m_eglContext {};
    EGLSurface m_eglSurface {};

    adt::VecBase<ImagePixelRGBA> m_vSurfaceBuffer {};
    adt::VecBase<ImagePixelRGBA> m_vTempBuff {};

    /* */

    ClientGL() = default;
    ClientGL(adt::IAllocator* pAlloc, const char* ntsName)
        : Client(pAlloc, ntsName) {}

    /* */

    virtual void start(int width, int height) override;
    virtual void swapBuffers() override;
    virtual void setSwapInterval(int interval) override;
    virtual void toggleVSync() override;
    virtual void bindContext() override;
    virtual void unbindContext() override;
    virtual void destroy() override;

    /* */

    void initGL();
};

} /* namespace platform::wayland */
