#include "app.hh"

#include "adt/defer.hh"

#if defined __linux__
    #include "platform/wayland/Client.hh"
    #if defined OPT_GL
        #include "platform/wayland/ClientGL.hh"
    #endif
#elif defined _WIN32
    #include "platform/win32/Window.hh"
#else
    #error "unsupported platform"
#endif

#if defined OPT_GL
    #include "render/gl/gl.hh"
#endif

#if defined OPT_SW
    #include "render/sw/sw.hh"
#endif

using namespace adt;

namespace app
{

WINDOW_TYPE g_eWindowType {};
RENDERER_TYPE g_eRendererType {};

IWindow* g_pWindow {};
render::IRenderer* g_pRenderer {};

static constexpr isize SCRATCH_SIZE = SIZE_1M;

/* NOTE: allocate scratch memory for each thread,
 * bacause using thread_local static buffer can actually cause stack overflow. */
adt::ThreadPoolWithMemory<128> g_threadPool {adt::StdAllocator::inst(), SCRATCH_SIZE};

IWindow*
allocWindow(IAllocator* pAlloc, const char* ntsName)
{
    static bool s_bAllocated = false;
    ADT_ASSERT_ALWAYS(s_bAllocated == false, "must not allocWindow() more than once");
    defer( s_bAllocated = true );

    switch (g_eWindowType)
    {
        default: break;

#ifdef __linux__
        case WINDOW_TYPE::WAYLAND_SHM:
        return pAlloc->alloc<platform::wayland::Client>(pAlloc, ntsName);

    #if defined OPT_GL
        case WINDOW_TYPE::WAYLAND_GL:
        return pAlloc->alloc<platform::wayland::ClientGL>(pAlloc, ntsName);
    #endif
#endif

#if defined _WIN32
        case WINDOW_TYPE::WINDOWS:
        return pAlloc->alloc<platform::win32::Window>(pAlloc, ntsName);
#endif
    }

    throw RuntimeException("failed to create the window");
}

render::IRenderer*
allocRenderer(IAllocator* pAlloc)
{
    static bool s_bAllocated = false;
    ADT_ASSERT_ALWAYS(s_bAllocated == false, "must not allocRenderer() more than once");
    defer( s_bAllocated = true );

    switch (g_eRendererType)
    {
        default: break;

#if defined OPT_SW
        case RENDERER_TYPE::SW:
        return pAlloc->alloc<render::sw::Renderer>();
#endif

#if defined OPT_GL
        case RENDERER_TYPE::OPEN_GL:
        return pAlloc->alloc<render::gl::Renderer>();
#endif
    }

    throw RuntimeException("failed to create a renderer");
}

} /* namespace app */;
