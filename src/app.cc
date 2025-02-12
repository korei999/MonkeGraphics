#include "app.hh"

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

using namespace adt;

namespace app
{

WINDOW_TYPE g_eWindowType {};
IWindow* g_pWindow {};

IWindow*
allocWindow(IAllocator* pAlloc, const char* ntsName)
{
    switch (g_eWindowType)
    {
        default: break;

#ifdef __linux__
        case WINDOW_TYPE::WAYLAND:
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

    return nullptr;
}

} /* namespace app */;
