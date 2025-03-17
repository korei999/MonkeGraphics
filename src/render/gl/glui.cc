#include "gl.hh"
#include "glui.hh"

#include "Text.hh"
#include "asset.hh"
#include "colors.hh"
#include "frame.hh"
#include "ui.hh"

using namespace adt;

namespace render::gl::ui
{

static Shader* s_pShTexMono;
static Text g_text;

void
init()
{
    s_pShTexMono = searchShader("QuadTexMonoBoxBlur");

    ttf::Font* pFont = asset::searchFont("assets/LiberationMono-Regular.ttf");
    ADT_ASSERT(pFont, " ");
    if (pFont)
    {
        g_rasterizer.rasterizeAscii(StdAllocator::inst(), pFont, 128.0f);
        g_texLiberation = Texture(g_rasterizer.m_altas.spanMono(), GL_LINEAR);
        g_text = Text(100);
    }
}

void
draw(Arena*)
{
    glDisable(GL_DEPTH_TEST);
    defer( glEnable(GL_DEPTH_TEST) );

    Shader* pShTex = s_pShTexMono;
    if (!pShTex) return;

    pShTex->use();
    g_text.bind();
    g_texLiberation.bind(GL_TEXTURE0);

    const math::M4 proj = math::M4Ortho(0, ::ui::WIDTH, 0, ::ui::HEIGHT, -10.0f, 10.0f);

    pShTex->setV4("u_color", V4From(colors::get(colors::WHITE), 0.75f));
    pShTex->setV2("u_texelSize", math::V2From(1.0f/g_texLiberation.m_width, 1.0f/g_texLiberation.m_height));

    /* fps */
    {
        pShTex->setM4("u_trm", proj * math::M4TranslationFrom({0.0f, ::ui::HEIGHT - 1.0f, -1.0f}));

        g_text.update(g_rasterizer, frame::g_sfFpsStatus);
        g_text.draw();
    }

    /* info */
    {
        StringView sv =
            "F: toggle fullscreen\n"
            "R: lock/unlock mouse\n"
            "Q/Escape: quit";

        int nSpaces = 0;
        for (auto ch : sv) if (ch == '\n') ++nSpaces;

        pShTex->setM4("u_trm", proj * math::M4TranslationFrom({0.0f, static_cast<f32>(nSpaces), -1.0f}));

        g_text.update(g_rasterizer, sv);
        g_text.draw();
    }

    /* ui */
    {
        for (const ::ui::Rect& rect : ::ui::g_poolRects)
        {
            if (bool(rect.eFlags & ::ui::Rect::FLAGS::NO_DRAW))
                continue;

            g_pShColor->use();
            g_pShColor->setM4("u_trm", proj *
                math::M4TranslationFrom({rect.x, rect.y, 0.0f}) *
                math::M4ScaleFrom({rect.width, rect.height, 0.0f})
            );
            g_pShColor->setV4("u_color", math::V4From(colors::get(colors::BLACK), 0.5f));
            g_quad.draw();

            if (bool(rect.eFlags & ::ui::Rect::FLAGS::TITLE))
            {
                pShTex->use();
                pShTex->setM4("u_trm", proj * math::M4TranslationFrom({rect.x, rect.y + rect.height - 1, 0.0f}));

                g_text.update(g_rasterizer, rect.sfTitle);
                g_text.draw();
            }
        }
    }
}

} /* namespace render::gl::ui */
