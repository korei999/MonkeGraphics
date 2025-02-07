#include "Client.hh"

#include "adt/OsAllocator.hh"
#include "adt/defer.hh"
#include "adt/logs.hh"

#include "shm.hh"

// #include <EGL/eglext.h>
// 
// static EGLint s_eglLastErrorCode = EGL_SUCCESS;
// 
// #ifndef NDEBUG
// #    define EGLD(C)                                                                                                    \
//         {                                                                                                              \
//             C;                                                                                                         \
//             if ((s_eglLastErrorCode = eglGetError()) != EGL_SUCCESS)                                                     \
//                 LOG_FATAL("eglLastErrorCode: {:#x}\n", s_eglLastErrorCode);                                              \
//         }
// #else
// #    define EGLD(C) C
// #endif

using namespace adt;

namespace platform::wayland
{

static const wl_registry_listener s_registryListener {
    .global = reinterpret_cast<decltype(wl_registry_listener::global)>(methodPointer(&Client::global)),
    .global_remove = reinterpret_cast<decltype(wl_registry_listener::global_remove)>(methodPointer(&Client::globalRemove)),
};

static const wl_shm_listener s_shmListener {
    .format = reinterpret_cast<decltype(wl_shm_listener::format)>(methodPointer(&Client::shmFormat))
};

static const xdg_wm_base_listener s_xdgWmBaseListener {
    .ping = reinterpret_cast<decltype(xdg_wm_base_listener::ping)>(methodPointer(&Client::xdgWmBasePing))
};

static const xdg_surface_listener s_xdgSurfaceListerer {
    .configure = reinterpret_cast<decltype(xdg_surface_listener::configure)>(methodPointer(&Client::xdgSurfaceConfigure))
};

static const xdg_toplevel_listener s_xdgTopLevelListener {
    .configure = reinterpret_cast<decltype(xdg_toplevel_listener::configure)>(methodPointer(&Client::xdgToplevelConfigure)),
    .close = reinterpret_cast<decltype(xdg_toplevel_listener::close)>(methodPointer(&Client::xdgToplevelClose)),
    .configure_bounds = reinterpret_cast<decltype(xdg_toplevel_listener::configure_bounds)>(methodPointer(&Client::xdgToplevelConfigureBounds)),
    .wm_capabilities = reinterpret_cast<decltype(xdg_toplevel_listener::wm_capabilities)>(methodPointer(&Client::xdgToplevelWmCapabilities)),
};

static const wl_seat_listener s_seatListener {
    .capabilities = reinterpret_cast<decltype(wl_seat_listener::capabilities)>(methodPointer(&Client::seatCapabilities)),
    .name = reinterpret_cast<decltype(wl_seat_listener::name)>(methodPointer(&Client::seatName)),
};

static const wl_keyboard_listener s_keyboardListener {
    .keymap = reinterpret_cast<decltype(wl_keyboard_listener::keymap)>(methodPointer(&Client::keyboardKeymap)),
    .enter = reinterpret_cast<decltype(wl_keyboard_listener::enter)>(methodPointer(&Client::keyboardEnter)),
    .leave = reinterpret_cast<decltype(wl_keyboard_listener::leave)>(methodPointer(&Client::keyboardLeave)),
    .key = reinterpret_cast<decltype(wl_keyboard_listener::key)>(methodPointer(&Client::keyboardKey)),
    .modifiers = reinterpret_cast<decltype(wl_keyboard_listener::modifiers)>(methodPointer(&Client::keyboardModifiers)),
    .repeat_info = reinterpret_cast<decltype(wl_keyboard_listener::repeat_info)>(methodPointer(&Client::keyboardRepeatInfo)),
};

static const wl_pointer_listener s_pointerListener {
    .enter = reinterpret_cast<decltype(wl_pointer_listener::enter)>(methodPointer(&Client::pointerEnter)),
    .leave = reinterpret_cast<decltype(wl_pointer_listener::leave)>(methodPointer(&Client::pointerLeave)),
    .motion = reinterpret_cast<decltype(wl_pointer_listener::motion)>(methodPointer(&Client::pointerMotion)),
    .button = reinterpret_cast<decltype(wl_pointer_listener::button)>(methodPointer(&Client::pointerButton)),
    .axis = reinterpret_cast<decltype(wl_pointer_listener::axis)>(methodPointer(&Client::pointerAxis)),
    .frame = reinterpret_cast<decltype(wl_pointer_listener::frame)>(methodPointer(&Client::pointerFrame)),
    .axis_source = reinterpret_cast<decltype(wl_pointer_listener::axis_source)>(methodPointer(&Client::pointerAxisSource)),
    .axis_stop = reinterpret_cast<decltype(wl_pointer_listener::axis_stop)>(methodPointer(&Client::pointerAxisStop)),
    .axis_discrete = reinterpret_cast<decltype(wl_pointer_listener::axis_discrete)>(methodPointer(&Client::pointerAxisDiscrete)),
    .axis_value120 = reinterpret_cast<decltype(wl_pointer_listener::axis_value120)>(methodPointer(&Client::pointerAxisValue120)),
    .axis_relative_direction = reinterpret_cast<decltype(wl_pointer_listener::axis_relative_direction)>(methodPointer(&Client::pointerAxisRelativeDirection)),
};

static const wl_output_listener s_outputListener {
    .geometry = reinterpret_cast<decltype(wl_output_listener::geometry)>(methodPointer(&Client::outputGeometry)),
    .mode = reinterpret_cast<decltype(wl_output_listener::mode)>(methodPointer(&Client::outputMode)),
    .done = reinterpret_cast<decltype(wl_output_listener::done)>(methodPointer(&Client::outputDone)),
    .scale = reinterpret_cast<decltype(wl_output_listener::scale)>(methodPointer(&Client::outputScale)),
    .name = reinterpret_cast<decltype(wl_output_listener::name)>(methodPointer(&Client::outputName)),
    .description = reinterpret_cast<decltype(wl_output_listener::description)>(methodPointer(&Client::outputDescription)),
};

static const zwp_relative_pointer_v1_listener s_relativePointerListener {
    .relative_motion = reinterpret_cast<decltype(zwp_relative_pointer_v1_listener::relative_motion)>(methodPointer(&Client::relativePointerMotion))
};

void
Client::start(int width, int height)
{
    LOG_GOOD("starting wayland client...\n");

    m_winWidth = m_width = width;
    m_winHeight = m_height = height;
    m_stride = m_width + 7; /* NOTE: simd padding */

    m_pDisplay = wl_display_connect(nullptr);
    if (!m_pDisplay)
        throw RuntimeException("wl_display_connect() failed");

    m_pRegistry = wl_display_get_registry(m_pDisplay);
    if (!m_pRegistry)
        throw RuntimeException("wl_display_get_registry() failed");

    wl_registry_add_listener(m_pRegistry, &s_registryListener, this);
    /* proc all the interfaces */
    wl_display_roundtrip(m_pDisplay);

    m_pSurface = wl_compositor_create_surface(m_pCompositor);
    if (!m_pSurface)
        throw RuntimeException("wl_compositor_create_surface() failed");

    m_pViewport = wp_viewporter_get_viewport(m_pViewporter, m_pSurface);
    if (!m_pViewport)
        throw RuntimeException("wp_viewporter_get_viewport() failed");

    wp_viewport_set_source(m_pViewport,
        wl_fixed_from_int(0), wl_fixed_from_int(0),
        wl_fixed_from_int(m_width), wl_fixed_from_int(m_height)
    );

    m_pXdgSurface = xdg_wm_base_get_xdg_surface(m_pXdgWmBase, m_pSurface);
    if (!m_pXdgSurface)
        throw RuntimeException("xdg_wm_base_get_xdg_surface() failed");

    xdg_surface_add_listener(m_pXdgSurface, &s_xdgSurfaceListerer, this);

    m_pXdgToplevel = xdg_surface_get_toplevel(m_pXdgSurface);
    if (!m_pXdgToplevel)
        throw RuntimeException("xdg_surface_get_toplevel() failed");

    xdg_toplevel_add_listener(m_pXdgToplevel, &s_xdgTopLevelListener, this);
    xdg_toplevel_set_title(m_pXdgToplevel, m_ntsName);
    wl_surface_commit(m_pSurface);

	/* Perform the initial commit and wait for the first configure event */
    wl_surface_commit(m_pSurface);
    while (wl_display_dispatch(m_pDisplay) != -1 && !m_bConfigured)
        ;

    wl_shm_add_listener(m_pShm, &s_shmListener, this);

    const int stride = m_stride * 4;
    const int shmPoolSize = m_height * stride;

    int fd = shm::allocFile(shmPoolSize);
    m_pPoolData = static_cast<u8*>(mmap(nullptr, shmPoolSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    m_poolSize = shmPoolSize;
    if (!m_pPoolData)
        throw RuntimeException("mmap() failed");

    m_vDepthBuffer.setSize(m_pAlloc, m_width * m_height);

    m_pShmPool = wl_shm_create_pool(m_pShm, fd, shmPoolSize);
    if (!m_pShmPool)
        throw RuntimeException("wl_shm_create_pool() failed");

    m_pBuffer = wl_shm_pool_create_buffer(m_pShmPool, 0, m_width, m_height, stride, WL_SHM_FORMAT_XRGB8888);
    if (!m_pBuffer)
        throw RuntimeException("wl_shm_pool_create_buffer() failed");

    m_pRelPointer = zwp_relative_pointer_manager_v1_get_relative_pointer(m_pRelPointerMgr, m_pPointer);
    if (!m_pRelPointer)
        throw RuntimeException("zwp_relative_pointer_manager_v1_get_relative_pointer() failed");

    zwp_relative_pointer_v1_add_listener(m_pRelPointer, &s_relativePointerListener, this);

    wl_display_roundtrip(m_pDisplay);

    if (m_bOpenGl)
        initEGL();
}

Span2D<ImagePixelARGB>
Client::surfaceBuffer()
{
    return {reinterpret_cast<ImagePixelARGB*>(m_pPoolData), m_width, m_height, m_stride};
}

void
Client::disableRelativeMode()
{
    m_bPointerRelativeMode = false;

    zwp_locked_pointer_v1_destroy(m_pLockedPointer);
    m_pLockedPointer = nullptr;
}

void
Client::enableRelativeMode()
{
    m_bPointerRelativeMode = true;

    hideCursor(true);

    m_pLockedPointer = zwp_pointer_constraints_v1_lock_pointer(
        m_pPointerConstraints, m_pSurface, m_pPointer, {},
        ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT
    );
}

void
Client::togglePointerRelativeMode()
{
    m_bPointerRelativeMode ? disableRelativeMode() : enableRelativeMode();

    LOG("relative mode: {}\n", m_bPointerRelativeMode);
}

void
Client::toggleFullscreen()
{
    m_bFullscreen ? unsetFullscreen() : setFullscreen();
}

void
Client::hideCursor([[maybe_unused]] bool bHide)
{
    LOG("hideCursor(): serial: {}\n", m_lastPointerEnterSerial);
    wl_pointer_set_cursor(m_pPointer, m_lastPointerEnterSerial, {}, 0, 0);
}

void
Client::setCursorImage([[maybe_unused]] String cursorType)
{
}

void
Client::setFullscreen()
{
    xdg_toplevel_set_fullscreen(m_pXdgToplevel, m_vOutputs.first());
    m_bFullscreen = true;
}

void
Client::unsetFullscreen()
{
    xdg_toplevel_unset_fullscreen(m_pXdgToplevel);
    m_bFullscreen = false;
}

void
Client::setSwapInterval(int interval)
{
//     m_swapInterval = interval;
//     EGLD( eglSwapInterval(m_eglDisplay, interval) );
//     LOG_NOTIFY("swapInterval: {}\n", m_swapInterval);
}

void
Client::toggleVSync()
{
    m_swapInterval == 1 ? setSwapInterval(0) : setSwapInterval(1);
}

void
Client::swapBuffers()
{
    // EGLD( eglSwapBuffers(m_eglDisplay, m_eglSurface) );
}

void
Client::procEvents()
{
    if (wl_display_dispatch(m_pDisplay) == -1)
        throw RuntimeException("wl_display_dispatch() failed");
}

void
Client::showWindow()
{
}

void
Client::destroy()
{
    if (m_pRegistry) wl_registry_destroy(m_pRegistry);
    if (m_pCompositor) wl_compositor_destroy(m_pCompositor);
    if (m_pSurface) wl_surface_destroy(m_pSurface);
    for (auto& output : m_vOutputs) wl_output_destroy(output);
    if (m_pShm) wl_shm_destroy(m_pShm);
    if (m_pShmPool) wl_shm_pool_destroy(m_pShmPool);
    if (m_pBuffer) wl_buffer_destroy(m_pBuffer);
    if (m_pSeat) wl_seat_destroy(m_pSeat);
    if (m_pKeyboard) wl_keyboard_destroy(m_pKeyboard);
    if (m_pPointer) wl_pointer_destroy(m_pPointer);
    if (m_pRelPointerMgr) zwp_relative_pointer_manager_v1_destroy(m_pRelPointerMgr);
    if (m_pRelPointer) zwp_relative_pointer_v1_destroy(m_pRelPointer);
    if (m_pLockedPointer) disableRelativeMode();
    if (m_pPointerConstraints) zwp_pointer_constraints_v1_destroy(m_pPointerConstraints);
    if (m_pXdgWmBase) xdg_wm_base_destroy(m_pXdgWmBase);
    if (m_pXdgSurface) xdg_surface_destroy(m_pXdgSurface);
    if (m_pXdgToplevel) xdg_toplevel_destroy(m_pXdgToplevel);
    if (m_pCallBack) wl_callback_destroy(m_pCallBack);
    if (m_pViewporter) wp_viewporter_destroy(m_pViewporter);
    if (m_pViewport) wp_viewport_destroy(m_pViewport);
    if (m_pDisplay) wl_display_disconnect(m_pDisplay);
    if (m_pPoolData) munmap(m_pPoolData, m_poolSize);

    *this = {};
}

void
Client::scheduleFrame()
{
    m_pCallBack = wl_surface_frame(m_pSurface);
    static const wl_callback_listener s_callbackListener {
        .done = reinterpret_cast<decltype(wl_callback_listener::done)>(methodPointer(&Client::callbackDone)),
    };
    wl_callback_add_listener(m_pCallBack, &s_callbackListener, this);
    updateSurface();
}

void
Client::bindGlContext()
{
    // EGLD ( eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext) );
}

void
Client::unbindGlContext()
{
    // EGLD( eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) );
}

void
Client::updateSurface()
{
    wl_surface_attach(m_pSurface, m_pBuffer, 0, 0);
    wl_surface_damage(m_pSurface, 0, 0, m_winWidth, m_winHeight);
    wl_surface_commit(m_pSurface);
}

void
Client::global(wl_registry* pRegistry, uint32_t name, const char* ntsInterface, uint32_t version)
{
    LOG("interface: '{}', version: {}, name: {}\n", ntsInterface, version, name);

    String sInterface = String(ntsInterface);

    if (sInterface == wl_compositor_interface.name)
    {
        m_pCompositor = static_cast<wl_compositor*>(wl_registry_bind(pRegistry, name, &wl_compositor_interface, version));
    }
    else if (sInterface ==  wl_shm_interface.name)
    {
        m_pShm = static_cast<wl_shm*>(wl_registry_bind(pRegistry, name, &wl_shm_interface, version));
    }
    else if (sInterface == xdg_wm_base_interface.name)
    {
        m_pXdgWmBase = static_cast<xdg_wm_base*>(wl_registry_bind(pRegistry, name, &xdg_wm_base_interface, version));
        xdg_wm_base_add_listener(m_pXdgWmBase, &s_xdgWmBaseListener, this);
    }
    else if (sInterface == wl_seat_interface.name)
    {
        m_pSeat = static_cast<wl_seat*>(wl_registry_bind(pRegistry, name, &wl_seat_interface, version));
        wl_seat_add_listener(m_pSeat, &s_seatListener, this);
    }
    else if (sInterface == wl_output_interface.name)
    {
        m_vOutputs.push(m_pAlloc,
            static_cast<wl_output*>(wl_registry_bind(pRegistry, name, &wl_output_interface, version))
        );
        wl_output_add_listener(m_vOutputs.last(), &s_outputListener, this);
    }
    else if (sInterface == wp_viewporter_interface.name)
    {
        m_pViewporter = static_cast<wp_viewporter*>(wl_registry_bind(pRegistry, name, &wp_viewporter_interface, version));
    }
    else if (sInterface == zwp_relative_pointer_manager_v1_interface.name)
    {
        m_pRelPointerMgr = static_cast<zwp_relative_pointer_manager_v1*>(wl_registry_bind(pRegistry, name, &zwp_relative_pointer_manager_v1_interface, version));
        if (!m_pRelPointerMgr)
            throw RuntimeException("failed to bind `zwp_relative_pointer_manager_v1_interface`");
    }
    else if (sInterface == zwp_pointer_constraints_v1_interface.name)
    {
        m_pPointerConstraints = static_cast<zwp_pointer_constraints_v1*>(wl_registry_bind(pRegistry, name, &zwp_pointer_constraints_v1_interface, version));
        if (!m_pPointerConstraints)
            throw RuntimeException("failed to bind `zwp_pointer_constraints_v1_interface`");
    }
}

void
Client::globalRemove(wl_registry*, uint32_t)
{
}

void
Client::shmFormat(wl_shm*, uint32_t)
{
}

void
Client::xdgWmBasePing(xdg_wm_base* pXdgWmBase, uint32_t serial)
{
    xdg_wm_base_pong(pXdgWmBase, serial);
}

void
Client::xdgSurfaceConfigure(xdg_surface* pXdgSurface, uint32_t serial)
{
    xdg_surface_ack_configure(pXdgSurface, serial);

    if (m_bConfigured)
        wl_surface_commit(m_pSurface);

    m_bConfigured = true;
}

void
Client::xdgToplevelConfigure(
    [[maybe_unused]] xdg_toplevel* pXdgToplevel,
    [[maybe_unused]] int32_t width,
    [[maybe_unused]] int32_t height,
    [[maybe_unused]] wl_array* pStates
)
{
    if (width > 0 && height > 0)
    {
        m_winWidth = width;
        m_winHeight = height;

        wp_viewport_set_destination(m_pViewport, width, height);
    }
}

void
Client::xdgToplevelClose(xdg_toplevel*)
{
    LOG_WARN("xdgToplevelClose()\n");
    m_bRunning = false;
}

void
Client::xdgToplevelConfigureBounds(
    [[maybe_unused]] xdg_toplevel*,
    [[maybe_unused]] int32_t width,
    [[maybe_unused]] int32_t height)
{
}

void
Client::xdgToplevelWmCapabilities(
    [[maybe_unused]] xdg_toplevel*,
    [[maybe_unused]] wl_array* pCapabilities
)
{
}

void
Client::seatCapabilities(wl_seat* pWlSeat, uint32_t capabilities)
{
    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        m_pPointer = wl_seat_get_pointer(pWlSeat);
        wl_pointer_add_listener(m_pPointer, &s_pointerListener, this);
    }
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        m_pKeyboard = wl_seat_get_keyboard(pWlSeat);
        wl_keyboard_add_listener(m_pKeyboard, &s_keyboardListener, this);
    }
}

void
Client::seatName(
    [[maybe_unused]] wl_seat* pWlSeat,
    [[maybe_unused]] const char* ntsName
)
{
    LOG("seatName: '{}'\n", ntsName);
}

void
Client::outputGeometry(
    [[maybe_unused]] wl_output* pOutput,
    [[maybe_unused]] int32_t x,
    [[maybe_unused]] int32_t y,
    [[maybe_unused]] int32_t physicalWidth,
    [[maybe_unused]] int32_t physicalHeight,
    [[maybe_unused]] int32_t subpixel,
    [[maybe_unused]] const char* ntsMake,
    [[maybe_unused]] const char* ntsModel,
    [[maybe_unused]] int32_t transform
)
{
    LOG("outputGeometry(): physicalWidth: {}, physicalHeight: {}, x: {}, y: {}, subpixel: {}, make: '{}', model: '{}', transform: {}\n",
        physicalHeight, physicalHeight, x, y, subpixel, ntsMake, ntsModel, transform);
}

void
Client::outputMode(
    [[maybe_unused]] wl_output* pOutput,
    [[maybe_unused]] uint32_t flags,
    [[maybe_unused]] int32_t width,
    [[maybe_unused]] int32_t height,
    [[maybe_unused]] int32_t refresh
)
{
    LOG("outputMode() width: {}, height: {}, refresh: {}\n", width, height, refresh);

    m_newWidth = width;
    m_newHeight = height;
}

void
Client::outputDone(wl_output*)
{
    LOG("outputDone()\n");
}

void
Client::outputScale(
    [[maybe_unused]] wl_output* pOutput,
    [[maybe_unused]] int32_t factor
)
{
    LOG("outputScale(): {}\n", factor);
    wl_surface_set_buffer_scale(m_pSurface, factor);
}

void
Client::outputName(
    [[maybe_unused]] wl_output* pOutput,
    [[maybe_unused]] const char* ntsName
)
{
    [[maybe_unused]] ssize idx = utils::search(m_vOutputs,
        [&](const wl_output* p) {
            if (pOutput == p)
                return true;
            else return false;
        }
    );

    LOG_NOTIFY("outputName() #{}: '{}'\n", idx, ntsName);
}

void
Client::outputDescription(
    [[maybe_unused]] wl_output* pOutput,
    [[maybe_unused]] const char* ntsDescription
)
{
    LOG("outputDescription(): '{}'\n", ntsDescription);
}

void
Client::callbackDone(
    [[maybe_unused]] wl_callback* pCallback,
    [[maybe_unused]] uint32_t callbackData
)
{
    wl_callback_destroy(pCallback);
    update();
}

void
Client::initEGL()
{
//     EGLD( m_eglDisplay = eglGetDisplay(m_pDisplay) );
//     if (m_eglDisplay == EGL_NO_DISPLAY)
//         LOG_FATAL("failed to create EGL display\n");
// 
//     EGLint major, minor;
//     if (!eglInitialize(m_eglDisplay, &major, &minor))
//         LOG_FATAL("failed to initialize EGL\n");
//     EGLD();
// 
//     /* Default is GLES */
//     if (!eglBindAPI(EGL_OPENGL_API))
//         LOG_FATAL("eglBindAPI(EGL_OPENGL_API) failed\n");
// 
//     LOG_OK("egl: major: {}, minor: {}\n", major, minor);
// 
//     EGLint count;
//     EGLD( eglGetConfigs(m_eglDisplay, nullptr, 0, &count) );
// 
//     EGLint configAttribs[] = {
//         EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
//         EGL_RED_SIZE, 8,
//         EGL_GREEN_SIZE, 8,
//         EGL_BLUE_SIZE, 8,
//         // EGL_ALPHA_SIZE, 8, /* KDE makes window transparent even in fullscreen */
//         EGL_DEPTH_SIZE, 24,
//         EGL_STENCIL_SIZE, 8,
//         EGL_CONFORMANT, EGL_OPENGL_BIT,
//         // EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
//         EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
//         // EGL_MIN_SWAP_INTERVAL, 0,
//         // EGL_MAX_SWAP_INTERVAL, 1,
//         // EGL_SAMPLE_BUFFERS, 1,
//         // EGL_SAMPLES, 4,
//         EGL_NONE
//     };
// 
//     EGLint n = 0;
//     Vec<EGLConfig> configs(OsAllocatorGet(), count);
//     defer( configs.destroy() );
//     configs.setSize(count);
// 
//     EGLD( eglChooseConfig(m_eglDisplay, configAttribs, configs.data(), count, &n) );
//     if (n == 0)
//         LOG_FATAL("Failed to choose an EGL config\n");
// 
//     EGLConfig eglConfig = configs[0];
// 
//     EGLint contextAttribs[] {
//         // EGL_CONTEXT_CLIENT_VERSION, 3,
//         // EGL_CONTEXT_MAJOR_VERSION, 3,
//         // EGL_CONTEXT_MINOR_VERSION, 3,
//         // EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
// #ifndef NDEBUG
//         EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
// #endif
//         EGL_NONE,
//     };
// 
//     EGLD( m_eglContext = eglCreateContext(m_eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs) );
// 
//     m_eglWindow = wl_egl_window_create(m_pSurface, m_width, m_height);
//     EGLD( m_eglSurface = eglCreateWindowSurface(m_eglDisplay, eglConfig, (EGLNativeWindowType)(m_eglWindow), nullptr) );
// 
//     wl_surface_commit(m_pSurface);
//     wl_display_roundtrip(m_pDisplay);
}

} /* namespace platform::wayland */
