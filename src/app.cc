#include "app.hh"

#ifdef __linux__
    #include "platform/wayland/Client.hh"
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
        default: throw RuntimeException("allocWindow(): can't create platform window");

        case WINDOW_TYPE::WAYLAND:
        return pAlloc->alloc<platform::wayland::Client>(ntsName);
    }
}

} /* namespace app */;
