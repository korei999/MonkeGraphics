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
StringFixed<128> g_sfFpsStatus;
f64 g_maxFps = 0.0;

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

static void
renderLoop(Arena* pArena)
{
    auto& win = app::windowInst();
    auto& renderer = app::rendererInst();

    VecManaged<f64> vFrameTimes(1000);
    defer( vFrameTimes.destroy() );
    f64 lastAvgFrameTimeUpdateTime {};

    f64 accumulator = 0.0;

    while (win.m_bRunning)
    {
        const f64 timer0 = utils::timeNowMS();

        {
            f64 newTime = timer0 / 1000.0;

            g_frameTime = newTime - g_time;

            accumulator += g_frameTime;
            g_time = newTime;

            win.procEvents();

            control::procInput();
            ui::updateState();

            while (accumulator >= g_dt)
            {
                control::g_camera.updatePos();

                if (!control::g_bPauseSimulation)
                {
                    game::updateState(pArena);
                    g_gameTime += g_dt;
                }

                accumulator -= g_dt;
            }

            renderer.draw(pArena);

            pArena->shrinkToFirstBlock();
            pArena->reset();
            win.swapBuffers();
        }

        f64 timer1 = utils::timeNowMS();

        if (g_maxFps > 0.0)
        {
            const f64 sleepFor = (1000.0/g_maxFps) - (timer1 - timer0);
            if (sleepFor > 0.0) utils::sleepMS(sleepFor);
        }

        timer1 = utils::timeNowMS();

        vFrameTimes.push(timer1 - timer0);

        if (timer1 > lastAvgFrameTimeUpdateTime + 1000.0)
        {
            f64 avg = 0;
            for (const f64 ft : vFrameTimes) avg += ft;

            char aBuff[128] {};
            isize n = print::toSpan(aBuff, "FPS: {} | avg frame time: {:.3} ms\n",
                vFrameTimes.size(), avg / vFrameTimes.size()
            );
            g_sfFpsStatus = StringView{aBuff, n};

            vFrameTimes.setSize(0);
            lastAvgFrameTimeUpdateTime = timer1;
        }
    }
}

static void
mainLoop()
{
    auto& win = app::windowInst();

    Arena frameArena {SIZE_1M}; /* reset inside renderLoop */
    defer( frameArena.freeAll() );

    win.showWindow();
    win.swapBuffers(); /* start events */

    win.togglePointerRelativeMode();
    win.toggleVSync();
    // win.toggleFullscreen();

    g_time = utils::timeNowS();

    game::updateState(&frameArena);

    renderLoop(&frameArena);
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
    defer(
        LOG_GOOD("cleaning up...\n");
        app::g_threadPool.destroy(StdAllocator::inst());
        renderer.destroy();

        isize nObj = asset::g_poolObjects.size();
        for (auto& asset : asset::g_poolObjects)
        {
            asset.destroy();
            --nObj;
        }
        ADT_ASSERT(nObj == 0, "nObj: {}", nObj);

        ui::destroy();
    );
}

} /* namespace frame */
