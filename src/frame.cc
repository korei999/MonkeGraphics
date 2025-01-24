#include "frame.hh"

#include "adt/logs.hh"
#include "adt/utils.hh"
#include "app.hh"
#include "draw.hh"

using namespace adt;

namespace frame
{

f64 g_currTime {};
f64 g_dt = FIXED_DELTA_TIME;
f64 g_gt {};

static void
fpsCounter()
{
    static f64 s_lastFPSTime = g_currTime;
    static int s_nFrames = 0;

    ++s_nFrames;

    if (g_currTime > s_lastFPSTime + 1000.0)
    {
        CERR("FPS: {}\n", s_nFrames);
        s_lastFPSTime = g_currTime;
        s_nFrames = 0;
    }
}

static void
refresh(void*)
{
    auto& win = *app::g_pWindow;
    Span2D sp = win.getSurfaceBuffer();

    draw::toBuffer(sp);
    fpsCounter();
}

void
start()
{
    auto& win = *app::g_pWindow;
    win.m_bRunning = true;
    g_currTime = utils::timeNowMS();

    win.regDrawCB(refresh, {});
    win.draw(); /* draw once to get events */

    f64 accumulator = 0.0;

    while (win.m_bRunning)
    {
        g_currTime = utils::timeNowMS();

        win.procEvents();

        while (accumulator >= g_dt)
        {
            /*game::updateState(&arena);*/
            g_gt += g_dt;
            accumulator -= g_dt;
        }
    }
}

} /* namespace frame */
