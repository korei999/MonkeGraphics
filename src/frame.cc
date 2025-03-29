#include "frame.hh"

#include "app.hh"
#include "control.hh"
#include "game/game.hh"
#include "ui.hh"
#include "asset.hh"

#include "adt/Vec.hh"
#include "adt/logs.hh"
#include "adt/defer.hh"

using namespace adt;

namespace frame
{

f64 g_time {};
f64 g_frameTime {};
const f64 g_dt = FIXED_DELTA_TIME;
f64 g_gameTime {};
adt::StringFixed<100> g_sfFpsStatus;

[[maybe_unused]] static void
refresh(void* pArg)
{
    Arena* pArena = static_cast<Arena*>(pArg);
    auto& renderer = app::rendererInst();

    static f64 s_accumulator = 0.0;

    f64 newTime = utils::timeNowS();
    g_frameTime = newTime - g_time;
    g_time = newTime;
    /*if (frameTime > 0.25)*/
    /*    frameTime = 0.25;*/

    s_accumulator += g_frameTime;

    while (s_accumulator >= g_dt)
    {
        game::updateState(pArena);
        g_gameTime += g_dt;
        s_accumulator -= g_dt;
    }

    renderer.draw(pArena);
}

static void
eventLoop()
{
#ifdef OPT_SW

    auto& win = app::window();
    auto& renderer = app::renderer();

    Arena frameArena(SIZE_1M);
    defer( frameArena.freeAll() );

    win.regUpdateCB(refresh, &frameArena);

    win.setFullscreen();
    win.enableRelativeMode();
    win.update(); /* get events */

    g_time = utils::timeNowS();

    while (win.m_bRunning)
    {
        win.procEvents();

        frameArena.shrinkToFirstBlock();
        frameArena.reset();
    }

#endif
}

static THREAD_STATUS
renderLoop(void* pArg)
{
    Arena* pArena = static_cast<Arena*>(pArg);

    app::allocScratchForThisThread(SIZE_1M);
    defer( app::destroyScratchForThisThread() );

    auto& win = app::windowInst();
    win.bindContext();

    auto& renderer = app::rendererInst();

    VecManaged<f64> vFrameTimes(StdAllocator::inst(), 1000);
    defer( vFrameTimes.destroy() );
    f64 lastAvgFrameTimeUpdateTime {};

    f64 accumulator = 0.0;

    while (win.m_bRunning)
    {
        const f64 t0 = utils::timeNowMS();

        {
            f64 newTime = utils::timeNowS();
            g_frameTime = newTime - g_time;
            g_time = newTime;

            accumulator += g_frameTime;

            control::procInput();

            while (accumulator >= g_dt)
            {
                game::updateState(pArena);
                g_gameTime += g_dt;
                accumulator -= g_dt;
            }

            renderer.draw(pArena);

            pArena->shrinkToFirstBlock();
            pArena->reset();
            win.swapBuffers();
        }

        const f64 t1 = utils::timeNowMS();
        vFrameTimes.push(t1 - t0);

        if (t1 > lastAvgFrameTimeUpdateTime + 1000.0)
        {
            f64 avg = 0;
            for (const f64 ft : vFrameTimes) avg += ft;

            char aBuff[128] {};
            ssize n = print::toBuffer(aBuff, sizeof(aBuff) - 1,
                "FPS: {} | avg frame time: {:.3} ms\n",
                vFrameTimes.size(), avg / vFrameTimes.size()
            );
            g_sfFpsStatus = StringView(aBuff, n);

            vFrameTimes.setSize(0);
            lastAvgFrameTimeUpdateTime = t1;
        }
    }

    return THREAD_STATUS(0);
}

static void
mainLoop()
{
    auto& win = app::windowInst();

    Arena frameArena(SIZE_8M); /* reset inside renderLoop */
    defer( frameArena.freeAll() );

    win.showWindow();
    win.swapBuffers(); /* start events */

    win.togglePointerRelativeMode();
    win.toggleVSync();
    // win.toggleFullscreen();

    g_time = utils::timeNowS();

    game::updateState(&frameArena);

    /* NOTE: Two threads, first for input events and UI, second for
     * input processing, game state updates and rendering.
     * Its done so that swapBuffers() doesn't block new input events. */

    win.unbindContext();
    Thread renderThread(renderLoop, &frameArena, Thread::ATTR::JOINABLE);
    defer( renderThread.join() );

    while (win.m_bRunning)
    {
        win.procEvents();
        ui::updateState();
    }
}

void
start()
{
    auto& win = app::windowInst();
    auto& renderer = app::rendererInst();

    win.m_bRunning = true;

    /* loadStuff() will need this */
    app::allocScratchForThisThread(SIZE_1K);
    defer( app::destroyScratchForThisThread() );

    game::loadStuff();
    win.bindContext();
    renderer.init();
    ui::init();

    switch (app::g_eWindowType)
    {
        case app::WINDOW_TYPE::WAYLAND_SHM:
        /* wayland event loop is not a normal render loop */
        eventLoop();
        break;

        default:
        mainLoop();
        break;
    }

    /* wait for running tasks */
    defer(
        LOG_GOOD("cleaning up...\n");
        app::g_threadPool.destroy();
        renderer.destroy();

        for (auto& asset : asset::g_poolObjects)
        asset.destroy();

        ui::destroy();
    );
}

} /* namespace frame */
