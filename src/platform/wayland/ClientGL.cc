#include "ClientGL.hh"

#include "adt/Array.hh"
#include "adt/logs.hh"

#include <EGL/eglext.h>

using namespace adt;

static EGLint s_eglLastErrorCode = EGL_SUCCESS;

#define EGLD(C)                                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        C;                                                                                                             \
        ADT_ASSERT_ALWAYS(s_eglLastErrorCode == EGL_SUCCESS, "eglLastErrorCode: 0x%08x", s_eglLastErrorCode);          \
    } while (0)

namespace platform::wayland
{

void
ClientGL::start(int width, int height)
{
    m_winWidth = m_width = width;
    m_winHeight = m_height = height;
    m_stride = m_width + 7; /* NOTE: simd padding */

    // wp_viewport_set_source(m_pViewport,
    //     wl_fixed_from_int(0), wl_fixed_from_int(0),
    //     wl_fixed_from_int(m_width), wl_fixed_from_int(m_height)
    // );

    initGL();
}

void
ClientGL::swapBuffers()
{
    EGLD( eglSwapBuffers(m_eglDisplay, m_eglSurface) );
};

void
ClientGL::setSwapInterval(int interval)
{
    m_swapInterval = interval;
    EGLD( eglSwapInterval(m_eglDisplay, interval) );
    LOG_NOTIFY("swapInterval: {}\n", m_swapInterval);
}

void
ClientGL::toggleVSync()
{
    m_swapInterval == 1 ? setSwapInterval(0) : setSwapInterval(1);
}

void
ClientGL::bindContext()
{
    EGLD ( eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext) );
}

void
ClientGL::unbindContext()
{
    EGLD( eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) );
}

void
ClientGL::destroy()
{
    Client::destroy();

    if (!eglDestroySurface(m_eglDisplay, m_eglSurface)) LOG_BAD("!eglDestroySurface\n");
    if (!eglDestroyContext(m_eglDisplay, m_eglContext)) LOG_BAD("!eglDestroyContext\n");
    if (!eglTerminate(m_eglDisplay)) LOG_BAD("!eglTerminate\n");
}

void
ClientGL::resizeCB(int width, int height)
{
    wl_egl_window_resize(m_eglWindow, width, height, 0, 0);
}

void
ClientGL::initGL()
{
    EGLD( m_eglDisplay = eglGetDisplay(m_pDisplay) );
    if (m_eglDisplay == EGL_NO_DISPLAY)
        LOG_FATAL("failed to create EGL display\n");

    EGLint major, minor;
    if (!eglInitialize(m_eglDisplay, &major, &minor))
        LOG_FATAL("failed to initialize EGL\n");
    EGLD();

    /* Default is GLES */
    if (!eglBindAPI(EGL_OPENGL_API))
        LOG_FATAL("eglBindAPI(EGL_OPENGL_API) failed\n");

    LOG_OK("egl: major: {}, minor: {}\n", major, minor);

    constexpr ssize MAX_COUNT = 100;

    EGLint count;
    EGLD( eglGetConfigs(m_eglDisplay, nullptr, 0, &count) );

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        // EGL_ALPHA_SIZE, 8, /* KDE makes window transparent even if fullscreen */
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_CONFORMANT, EGL_OPENGL_BIT,
        // EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        // EGL_MIN_SWAP_INTERVAL, 0,
        // EGL_MAX_SWAP_INTERVAL, 1,
        // EGL_SAMPLE_BUFFERS, 1,
        // EGL_SAMPLES, 4,
        EGL_NONE
    };

    EGLint n = 0;
    Array<EGLConfig, MAX_COUNT> aConfigs {};
    ssize maxCount = utils::min(MAX_COUNT, static_cast<ssize>(count));
    aConfigs.setSize(maxCount);

    EGLD( eglChooseConfig(m_eglDisplay, configAttribs, aConfigs.data(), maxCount, &n) );
    ADT_ASSERT_ALWAYS(n != 0, "Failed to choose an EGL config\n");

    EGLConfig eglConfig = aConfigs[0];

    EGLint contextAttribs[] {
        // EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
#ifndef NDEBUG
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
#endif
        EGL_NONE,
    };

    EGLD( m_eglContext = eglCreateContext(m_eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs) );

    m_eglWindow = wl_egl_window_create(m_pSurface, m_width, m_height);
    EGLD( m_eglSurface = eglCreateWindowSurface(m_eglDisplay, eglConfig, (EGLNativeWindowType)(m_eglWindow), nullptr) );

    LOG_GOOD("wayland egl client started...\n");
}

} /* namespace platform::wayland */
