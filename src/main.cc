#include "adt/String.hh"
#include "adt/FreeList.hh"
#include "adt/defer.hh"

#include "app.hh"
#include "frame.hh"

#include "adt/math.hh"

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

    V2Base v2 {1.1f, 2.2f};
    V2Base<i32> iv2 = V2Base<i32>(v2);

    COUT("v2: [{}]\n", v2);
    COUT("iv2: [{}]\n", iv2);

    V3Base v3 {1.11f, 2.22f, 3.33f};

    COUT("v3: [{}]\n", v3);
    COUT("v3 xyz: [{}, {}, {}]\n", v3.x(), v3.y(), v3.z());

    V4Base v4 {1.11f, 2.22f, 3.33f, 4.44f};

    COUT("v4: [{}]\n", v4);
    COUT("v4 xy zw: [{}, {}]\n", v4.xy(), v4.zw());

    V4Base<i32> iv4FromV4 = v4;

    COUT("iv4FromV4: [{}]\n", iv4FromV4);

    M4Base m4 {INIT};

    COUT("m4: {}\n", m4);

    M3 m3FromM4 = m4;

    COUT("m3FromM4: {}\n", m3FromM4);

    {
        V2Base v0 {1.1f, 2.2f};
        V2Base v1 {1.0f, 2.0f};

        COUT("V2+: {}\n", v0 + v1);
        COUT("V2-: {}\n", v0 - v1);
        COUT("V2*: {}\n", v0 * v1);
        COUT("V2/: {}\n", v0 / v1);

        COUT("\n");
    }

    {
        V3 v0 {1.1f, 2.2f, 3.3f};
        V3 v1 = v0 + v0;

        COUT("v0: {}\n", v0);
        COUT("v1: {}\n", v1);

        v0 -= v1;
        v0 += v1;
        COUT("v0 -=: {}\n", v0);

        COUT("\n");
    }

    {
        V4Base v0 {1.1f, 2.2f, 3.3f, 4.4f};
        V4Base v1 = v0 + v0;

        COUT("v0: {}\n", v0);
        COUT("v1: {}\n", v1);
    }

    {
        M4Base m0 = M4Iden();
        M4Base m1 = M4Sca(10.0f);
        M4Base m2 = M4Tra(2.0f, 3.0f, 4.0f);

        M4Base mUninit;

        COUT("mUninit: {}\n", mUninit);

        COUT("m0: {}\n", m0);
        COUT("m1: {}\n", m1);

        COUT("m0 * m2: {}\n", m0 * m2);

        COUT("m0[3, 3]: {}\n", m0[3][3]);

        COUT("\n");
    }

    COUT("\n");
    exit(0);
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
