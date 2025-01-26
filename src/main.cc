#include "adt/String.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"

#include "app.hh"
#include "frame.hh"

using namespace adt;

static void
parseArgs(int argc, char** argv)
{
    for (ssize i = 0; i < argc; ++i)
    {
        String sArg = argv[i];

        if (sArg.endsWith("--"))
        {
            // add args...
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
        app::g_pWindow->start(400, 300);
        defer( app::g_pWindow->destroy() );

        frame::start();
    }
    catch (IException& ex)
    {
        ex.logErrorMsg(stdout);
    }
}
