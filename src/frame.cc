#include "frame.hh"

#include "app.hh"
#include "control.hh"
#include "game/game.hh"
#include "ui.hh"

#include "adt/Vec.hh"
#include "adt/logs.hh"
#include "adt/defer.hh"

#if !defined NDEBUG
    #include "asset.hh"
#endif

using namespace adt;

namespace frame
{

f64 g_time {};
f64 g_frameTime {};
const f64 g_dt = FIXED_DELTA_TIME;
f64 g_gameTime {};

static void
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

    control::procInput();

    while (s_accumulator >= g_dt)
    {
        game::updateState(pArena);
        g_gameTime += g_dt;
        s_accumulator -= g_dt;
    }

    renderer.drawGame(pArena);
}

static void
eventLoop()
{
#ifdef OPT_SW

    auto& win = app::window();
    auto& renderer = app::renderer();

    Arena frameArena(SIZE_8M);
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

static void
mainLoop()
{
    auto& win = app::windowInst();

    Arena frameArena(SIZE_8M);
    defer( frameArena.freeAll() );

    win.showWindow();
    win.swapBuffers(); /* start events */

    win.togglePointerRelativeMode();
    win.toggleVSync();
    // win.toggleFullscreen();

    g_time = utils::timeNowS();

    VecManaged<f64> vFrameTimes(StdAllocator::inst(), 1000);
    defer( vFrameTimes.destroy() );
    f64 lastAvgFrameTimeUpdateTime {};

    game::updateState(&frameArena);

    while (win.m_bRunning)
    {
        const f64 t0 = utils::timeNowMS();

        {
            win.procEvents();

            refresh(&frameArena);

            frameArena.shrinkToFirstBlock();
            frameArena.reset();
            win.swapBuffers();
        }

        const f64 t1 = utils::timeNowMS();
        vFrameTimes.push(t1 - t0);

        if (t1 > lastAvgFrameTimeUpdateTime + 1000.0)
        {
            f64 avg = 0;
            for (f64 ft : vFrameTimes) avg += ft;

            CERR("FPS: {} | avg frame time: {} ms\n", vFrameTimes.size(), avg / vFrameTimes.size());
            vFrameTimes.setSize(0);
            lastAvgFrameTimeUpdateTime = t1;
        }
    }
}

void
start()
{
    auto& win = app::windowInst();
    auto& renderer = app::rendererInst();

    win.m_bRunning = true;

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
    renderer.destroy();

#ifndef NDEBUG

    for (auto& asset : asset::g_poolObjects)
        asset.destroy();

#endif
}

} /* namespace frame */
