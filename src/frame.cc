#include "frame.hh"

#include "app.hh"
#include "asset.hh"
#include "control.hh"
#include "draw.hh"
#include "game.hh"
#include "gl.hh"

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

    draw::toSurfaceBuffer(pArena);
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
}

static void
mainLoop()
{
#if defined OPT_GL

    auto& win = app::window();
    win.m_bRunning = true;
    g_time = utils::timeNowS();

    Arena frameArena(SIZE_8M);
    defer( frameArena.freeAll() );

    game::loadAssets();

    win.bindContext();
    auto spSurface = win.surfaceBuffer();

    gl::init();
    gl::loadShaders();

    win.swapBuffers(); /* trigger events */

    win.togglePointerRelativeMode();
    win.toggleVSync();

    gl::Texture surfaceTexture(spSurface.getStride(), spSurface.getHeight());
    gl::Shader* pshQuad = gl::searchShader("Quad");
    gl::Quad quad(adt::INIT);
    ADT_ASSERT(pshQuad, " ");

    pshQuad->queryActiveUniforms();

    while (win.m_bRunning)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        win.procEvents();
        refresh(&frameArena);

        surfaceTexture.bind(GL_TEXTURE0);
        surfaceTexture.subImage(win.surfaceBuffer());
        pshQuad->use();
        quad.bind();
        quad.draw();

        frameArena.shrinkToFirstBlock();
        frameArena.reset();
        win.swapBuffers();
    }

#endif
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
        mainLoop();
        break;
    }

#ifndef NDEBUG

    for (auto& asset : asset::g_objects)
        asset.destroy();

    #if defined OPT_GL

    for (auto& shader : gl::g_shaders)
        shader.destroy();

    #endif

#endif
}

} /* namespace frame */
