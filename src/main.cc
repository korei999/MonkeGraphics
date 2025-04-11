#include "adt/String.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"

#include "app.hh"
#include "frame.hh"

#include "adt/math2.hh"

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

static void
what()
{
    using namespace adt::math;

    M3 m3 = M3::iden();
    M4 m4 = M4::iden();

    LOG_BAD("m3: {}\n", m3);
    LOG_BAD("m4: {}\n", m4);
}

int
main(int argc, char** argv)
{
    what();

    return startup(argc, argv);
}

#endif

static void
parseArgs(int argc, char** argv)
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
startup(int argc, char** argv)
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
        FreeList allocator(SIZE_1K);
        defer( allocator.freeAll() );

        const char* ntsName = "MonkeGraphics";

        /* allocate polymorphic singletons */
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
