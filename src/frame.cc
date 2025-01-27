#include "frame.hh"

#include "app.hh"
#include "control.hh"
#include "draw.hh"

#include "adt/logs.hh"
#include "adt/utils.hh"

using namespace adt;

namespace frame
{

f64 g_time {};
f64 g_frameTime {};
f64 g_dt = FIXED_DELTA_TIME;
f64 g_gt {};

static void
fpsCounter()
{
    static f64 s_lastFPSTime = g_time;
    static int s_nFrames = 0;

    ++s_nFrames;

    if (g_time > s_lastFPSTime + 1000.0)
    {
        CERR("FPS: {}\n", s_nFrames);
        s_lastFPSTime = g_time;
        s_nFrames = 0;
    }
}

static void
refresh(void*)
{
    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();

    static f64 accumulator = 0.0;

    f64 newTime = utils::timeNowMS();
    f64 frameTime = newTime - g_time;
    g_frameTime = frameTime;
    g_time = newTime;
    if (frameTime > 0.25)
        frameTime = 0.25;

    accumulator += frameTime;

    control::procKeys();

    while (accumulator >= g_dt)
    {
        /*game::updateState(&arena);*/
        g_gt += g_dt;
        accumulator -= g_dt;
    }

    draw::toBuffer();
    fpsCounter();
}

void
start()
{
    auto& win = *app::g_pWindow;
    win.m_bRunning = true;
    g_time = utils::timeNowMS();

    win.regDrawCB(refresh, {});
    win.draw(); /* draw once to get events */

    while (win.m_bRunning)
    {
        win.procEvents();
    }
}

} /* namespace frame */
