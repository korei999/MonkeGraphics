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
    return startup(argc, argv);
}

#endif

static void
parseArgs(const int argc, const char* const argv[])
{
    for (isize i = 1; i < argc; ++i)
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
        ThreadPool threadPool {SIZE_1K, SIZE_1M * 128};
        IThreadPool::setGlobal(&threadPool);
        defer( threadPool.destroy() );

        Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4, true};
        ILogger::setGlobal(&logger);
        defer( logger.destroy() );

        const char* ntsName = "MonkeGraphics";

        app::g_pWindow = app::allocWindow(StdAllocator::inst(), ntsName);
        app::g_pRenderer = app::allocRenderer(StdAllocator::inst());

        app::g_pWindow->start(1280, 720);
        defer( app::g_pWindow->destroy() );

        frame::start();
    }
    catch (const std::exception& ex)
    {
        LogError{"{}\n", ex.what()};
    }

    return 0;
}
