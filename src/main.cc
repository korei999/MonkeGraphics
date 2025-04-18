#include "app.hh"
#include "frame.hh"

#include "adt/String.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"

using namespace adt;

static int startup(int argc, char** argv);

#if defined _WIN32 && defined NDEBUG

int WINAPI
WinMain(
    [[maybe_unused]] HINSTANCE instance,
    [[maybe_unused]] HINSTANCE previnstance,
    [[maybe_unused]] LPSTR cmdline,
    [[maybe_unused]] int cmdshow)
{
    return startup(__argc, __argv);
}

#else

int
main(int argc, char** argv)
{
    return startup(argc, argv);
}

#endif

static void
parseArgs(const int argc, const char* const argv[])
{
    for (ssize i = 1; i < argc; ++i)
    {
        const StringView svArg = argv[i];

        if (svArg.beginsWith("--"))
        {
            if (svArg == "--wayland-shm")
            {
                app::g_eWindowType = app::WINDOW_TYPE::WAYLAND_SHM;
                app::g_eRendererType = app::RENDERER_TYPE::SW;
            }
            else if (svArg == "--wayland-gl")
            {
                app::g_eWindowType = app::WINDOW_TYPE::WAYLAND_GL;
                app::g_eRendererType = app::RENDERER_TYPE::OPEN_GL;
            }
            else if (svArg == "--windows")
            {
                app::g_eWindowType = app::WINDOW_TYPE::WINDOWS;
                app::g_eRendererType = app::RENDERER_TYPE::OPEN_GL;
            }
        }
        else return;
    }
}

static int
startup(int argc, char* argv[])
{
#if defined __linux__
    app::g_eWindowType = app::WINDOW_TYPE::WAYLAND_GL;
#elif defined _WIN32
    app::g_eWindowType = app::WINDOW_TYPE::WINDOWS;
#endif

    app::g_eRendererType = app::RENDERER_TYPE::OPEN_GL;

    parseArgs(argc, argv);

    try
    {
        FreeList allocator {SIZE_1K};
        defer( allocator.freeAll() );

        const char* ntsName = "MonkeGraphics";

        app::g_pWindow = app::allocWindow(&allocator, ntsName);
        app::g_pRenderer = app::allocRenderer(&allocator);

        app::g_pWindow->start(1280, 720);
        defer( app::g_pWindow->destroy() );

        frame::start();
    }
    catch (IException& ex)
    {
        ex.printErrorMsg(stdout);
    }

    return 0;
}
