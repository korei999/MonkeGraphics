#include "frame.hh"

#include "app.hh"
#include "asset.hh"
#include "control.hh"
#include "draw.hh"
#include "game.hh"

#include "adt/utils.hh"

using namespace adt;

namespace frame
{

f64 g_time {};
f64 g_frameTime {};
constexpr f64 g_dt = FIXED_DELTA_TIME;
f64 g_gameTime {};

static void
refresh(void* pArg)
{
    Arena* pArena = static_cast<Arena*>(pArg);

    static f64 s_accumulator = 0.0;

    f64 newTime = utils::timeNowS();
    f64 frameTime = newTime - g_time;
    g_frameTime = frameTime;
    g_time = newTime;
    /*if (frameTime > 0.25)*/
    /*    frameTime = 0.25;*/

    s_accumulator += frameTime;

    control::procInput();

    while (s_accumulator >= g_dt)
    {
        game::updateState(pArena);
        g_gameTime += g_dt;
        s_accumulator -= g_dt;
    }

    draw::toBuffer(pArena);
}

static void
eventLoop()
{
    auto& win = app::window();
    win.m_bRunning = true;
    g_time = utils::timeNowS();

    game::loadAssets();

    Arena frameArena(SIZE_8M);
    defer( frameArena.freeAll() );

    win.regUpdateCB(refresh, &frameArena);

    // win.setFullscreen();
    win.enableRelativeMode();
    win.update(); /* get events */

    g_time = utils::timeNowS();

    while (win.m_bRunning)
    {
        win.procEvents();

        frameArena.shrinkToFirstBlock();
        frameArena.reset();
    }

    for (auto& asset : asset::g_objects)
        asset.destroy();
    asset::g_objects.destroy();
}

static void
renderLoop()
{
    auto& win = app::window();
    win.m_bRunning = true;
    g_time = utils::timeNowS();

    Arena arena(SIZE_8M);
    defer( arena.freeAll() );
}

void
start()
{
    switch (app::g_eWindowType)
    {
        case app::WINDOW_TYPE::WAYLAND:
        /* wayland event loop is not a normal render loop */
        eventLoop();
        break;

        default:
        renderLoop();
        break;
    }
}

} /* namespace frame */
