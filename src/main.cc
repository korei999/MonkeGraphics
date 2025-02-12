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
            if (svArg == "--wayland")
                app::g_eWindowType = app::WINDOW_TYPE::WAYLAND;
            else if (svArg == "--wayland-gl")
                app::g_eWindowType = app::WINDOW_TYPE::WAYLAND_GL;
            else if (svArg == "--windows")
                app::g_eWindowType = app::WINDOW_TYPE::WINDOWS;
        }
        else return;
    }
}

static int
startup(int argc, char** argv)
{
#ifdef __linux__
    app::g_eWindowType = app::WINDOW_TYPE::WAYLAND;
#elif defined _WIN32
    app::g_eWindowType = app::WINDOW_TYPE::WINDOWS;
#endif

    parseArgs(argc, argv);

    try
    {
        FreeList allocator(SIZE_1K);
        defer( allocator.freeAll() );

        const char* ntsName = "MonkeGraphics";

        app::g_pWindow = app::allocWindow(&allocator, ntsName);

        ADT_ASSERT_ALWAYS(
            app::g_pWindow, "allocWindow() failed: app::g_pWindow: %p",
            reinterpret_cast<void*>(app::g_pWindow)
        );

        app::g_pWindow->start(640, 480);
        defer( app::g_pWindow->destroy() );

        frame::start();
    }
    catch (IException& ex)
    {
        ex.printErrorMsg(stdout);
    }

    return 0;
}
