#include "adt/String.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"

#include "app.hh"
#include "frame.hh"

using namespace adt;

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

int
main(int argc, char** argv)
{
    app::g_eWindowType = app::WINDOW_TYPE::WAYLAND;

    parseArgs(argc, argv);

    try
    {
        FreeList allocator(SIZE_1K);
        defer( allocator.freeAll() );

        const char* ntsName = "MonkeGraphics";

        app::g_pWindow = app::allocWindow(&allocator, ntsName);
        if (!app::g_pWindow)
            throw RuntimeException("allocWindow() failed: app::g_pWindow == nullptr");

        app::g_pWindow->start(640, 480);
        defer( app::g_pWindow->destroy() );

        frame::start();
    }
    catch (IException& ex)
    {
        ex.printErrorMsg(stdout);
    }
}
