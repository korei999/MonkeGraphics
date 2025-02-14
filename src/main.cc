#include "adt/String.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"

#include "app.hh"
#include "frame.hh"

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
    startup(argc, argv);
}

#endif

static void
parseArgs(int argc, char** argv)
{
    for (ssize i = 1; i < argc; ++i)
    {
        const String svArg = argv[i];

        if (svArg.beginsWith("--"))
        {
            if (svArg == "--wayland-shm")
            {
                app::g_eWindowType = app::WINDOW_TYPE::WAYLAND;
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
startup(int argc, char** argv)
{
#ifdef __linux__
    app::g_eWindowType = app::WINDOW_TYPE::WAYLAND_GL;
#elif defined _WIN32
    app::g_eWindowType = app::WINDOW_TYPE::WINDOWS;
#endif

    app::g_eRendererType = app::RENDERER_TYPE::OPEN_GL;

    parseArgs(argc, argv);

    try
    {
        FreeList allocator(SIZE_1K);
        defer( allocator.freeAll() );

        const char* ntsName = "MonkeGraphics";

        app::g_pWindow = app::allocWindow(&allocator, ntsName);
        app::g_pRenderer = app::allocRenderer(&allocator);

        if (!app::g_pWindow)
        {
            CERR("failed to create platform window\n");
            return 1;
        }

        if (!app::g_pRenderer)
        {
            CERR("failed to create renderer\n");
            return 1;
        }

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
