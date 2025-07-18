#include "gl.hh"
#include "glui.hh"

#include "control.hh"
#include "app.hh"
#include "Text.hh"
#include "asset.hh"
#include "colors.hh"
#include "frame.hh"
#include "ui.hh"

using namespace adt;

namespace render::gl::ui
{

struct DrawCommand
{
    void (*pfn)(void* pArg) {};
    void* pArg {};
};

static ::ui::Offset drawArrowList(
    Arena* pArean,
    Vec<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    const ::ui::Offset off
);

static Shader* s_pShTexMonoBlur;
static Shader* s_pShTex;
static Text s_text;
static ttf::Rasterizer s_rastLiberation;
static Texture s_texLiberation;
static Quad s_quad0to1;

void
init()
{
    s_pShTexMonoBlur = searchShader("QuadTexMonoBoxBlur");
    s_pShTex = searchShader("QuadTexColor");

    ttf::Font* pFont = asset::searchFont("assets/LiberationMono-Regular.ttf");
    ADT_ASSERT(pFont, " ");
    if (pFont)
    {
        s_rastLiberation.rasterizeAscii(StdAllocator::inst(), pFont, 64.0f);
        new(&s_texLiberation) Texture {s_rastLiberation.m_altas.spanMono(), GL_LINEAR};
        new(&s_text) Text {255};
    }

    s_quad0to1 = Quad(INIT, Quad::TYPE::ZERO_TO_ONE);
}

static ::ui::Offset
drawText(
    Arena* pArena,
    Vec<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const StringView sv,
    const math::V4 fgColor,
    const math::M4& proj,
    const ::ui::Offset off
)
{
    auto clDraw = [&widget, sv, proj, fgColor, off]
    {
        s_texLiberation.bind(GL_TEXTURE0);
        s_pShTexMonoBlur->use();

        s_pShTexMonoBlur->setM4("u_trm", proj *
            math::M4TranslationFrom({widget.x + off.x, widget.y + off.y, -1.0f})
        );
        s_pShTexMonoBlur->setV4("u_color", fgColor);

        s_text.update(s_rastLiberation, &app::g_threadPool.scratchBuffer(), sv, true);
        s_text.draw();
    };

    auto* pClDraw = pArena->alloc<decltype(clDraw)>(clDraw);

    pVCommands->emplace(pArena, +[](void* p)
        {
            auto* pCl = static_cast<decltype(clDraw)*>(p);
            pCl->operator()();
        },
        pClDraw
    );

    return {static_cast<int>(sv.size()), 1};
}

static ::ui::Offset
drawMenu(
    Arena* pArena,
    Vec<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    const ::ui::Offset off
)
{
    ADT_ASSERT(entry.m_eType == ::ui::Entry::TYPE::MENU, "");

    auto& menu = entry.m_menu;

    ::ui::Offset thisOff {0, 0};

    {
        auto xy = drawText(pArena, pVCommands, widget, menu.sfName, menu.color, proj, off);
        thisOff.x = utils::max(xy.x, thisOff.x);
        thisOff.y += xy.y;
    }

    for (const ::ui::Entry& child : menu.vEntries)
    {
        const isize idx = menu.vEntries.idx(&child);
        const math::V4 col = [&]
        {
            if (idx == menu.selectedI) return menu.selColor;
            else return menu.color;
        }();

        ::ui::Offset xy {};

        switch (child.m_eType)
        {
            case ::ui::Entry::TYPE::TEXT:
            {
                xy = drawText(pArena, pVCommands, widget, child.m_text.sfName, col, proj, {off.x + 2, off.y + thisOff.y});
            }
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            {
                xy = drawArrowList(pArena, pVCommands, widget, child, proj, {off.x + 2, off.y + thisOff.y});
            }
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                xy = drawMenu(pArena, pVCommands, widget, child, proj, {off.x + 2, off.y + thisOff.y});
            }
            break;
        }

        /* +2 since children are off by 2 */
        thisOff.x = utils::max(xy.x + 2, thisOff.x);
        thisOff.y += xy.y;
    }

    return thisOff;
}

static ::ui::Offset
drawArrowList(
    Arena* pArena,
    Vec<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    const ::ui::Offset off
)
{
    ADT_ASSERT(entry.m_eType == ::ui::Entry::TYPE::ARROW_LIST, "");

    auto& arrowList = entry.m_arrowList;

    ::ui::Offset thisOff {0, 0};

    {
        drawText(pArena, pVCommands, widget, "<", entry.m_arrowList.arrowColor, proj, off);
        auto xy = drawText(pArena, pVCommands, widget, arrowList.sfName, arrowList.color, proj, {off.x + 1, off.y});
        drawText(pArena, pVCommands, widget, ">", entry.m_arrowList.arrowColor, proj, {off.x + xy.x + 1, off.y});

        thisOff.x = utils::max(thisOff.x, xy.x + 2);
        ++thisOff.y;
    }

    if (entry.m_arrowList.vEntries.size() > 0)
    {
        const auto& sel = entry.m_arrowList.vEntries[entry.m_arrowList.selectedI];
        switch (sel.m_eType)
        {
            case ::ui::Entry::TYPE::TEXT:
            {
                auto xy = drawText(pArena, pVCommands, widget, sel.m_text.sfName, sel.m_text.color, proj, {off.x + 2, off.y + thisOff.y});

                thisOff.x = utils::max(thisOff.x, xy.x);
                thisOff.y += xy.y;
            }
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                auto xy = drawMenu(pArena, pVCommands, widget, sel, proj, {off.x, off.y + thisOff.y});

                thisOff.x = utils::max(thisOff.x, xy.x);
                thisOff.y += xy.y;
            }
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            {
            }
            break;
        }
    }

    return thisOff;
}

static void
drawWidget(Arena* pArena, Vec<DrawCommand>* pVCommands, ::ui::Widget* pWidget, const math::M4& proj)
{
    ::ui::Offset off {0, 0};
    ::ui::Offset thisOff {0, 0};

    if (bool(pWidget->eFlags & ::ui::Widget::FLAGS::TITLE))
    {
        ::ui::Offset xy = drawText(pArena, pVCommands, *pWidget, pWidget->sfTitle,
            V4From(colors::WHITE, 0.75f), proj, off
        );
        thisOff.x = utils::max(thisOff.x, xy.x);
        thisOff.y += xy.y;
    }

    for (const auto& entry : pWidget->vEntries)
    {
        switch (entry.m_eType)
        {
            case ::ui::Entry::TYPE::ARROW_LIST:
            {
                auto xy = drawArrowList(pArena, pVCommands, *pWidget, entry, proj, {off.x, off.y + thisOff.y});
                thisOff.x = utils::max(thisOff.x, xy.x);
                thisOff.y += xy.y;
            }
            break;

            case ::ui::Entry::TYPE::TEXT:
            {
                auto xy = drawText(pArena, pVCommands, *pWidget, entry.m_text.sfName, entry.m_text.color, proj, {off.x, off.y + thisOff.y});
                thisOff.x = utils::max(thisOff.x, xy.x);
                thisOff.y += xy.y;
            }
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                auto xy = drawMenu(pArena, pVCommands, *pWidget, entry, proj, {off.x, off.y + thisOff.y});
                thisOff.x = utils::max(thisOff.x, xy.x);
                thisOff.y += xy.y;
            }
            break;
        }
    }

    pWidget->priv.grabWidth = thisOff.x;
    pWidget->priv.grabHeight = thisOff.y;

    // constexpr f32 uiWidthToUiHeightInv = 1.0f / (::ui::WIDTH / ::ui::HEIGHT);

    /* bg rectangle */
    if (pWidget->priv.grabHeight > 0 && pWidget->priv.grabWidth > 0)
    {
        g_pShColor->use();
        g_pShColor->setM4("u_trm",
            proj *
            math::M4TranslationFrom({
                pWidget->x - pWidget->border,
                pWidget->y - pWidget->border /* *uiWidthToUiHeightInv */,
                -5.0f
            }) *
            math::M4ScaleFrom({
                pWidget->priv.grabWidth + pWidget->border*2,
                pWidget->priv.grabHeight + pWidget->border*2 /* *uiWidthToUiHeightInv */,
                0.0f
            })
        );
        g_pShColor->setV4("u_color", pWidget->bgColor);
        g_quad.draw();
    }
}

static void
drawWidgets(Arena* pArena, Vec<DrawCommand>* pVCommands, const math::M4& proj)
{
    for (::ui::Widget& widget : ::ui::g_poolWidgets)
    {
        if (bool(widget.eFlags & ::ui::Widget::FLAGS::NO_DRAW))
            continue;

        drawWidget(pArena, pVCommands, &widget, proj);
    }
}

void
draw(Arena* pArena)
{
    /* Save drawText commands in the buffer. Draw them over the rectangle later. */
    Vec<DrawCommand> vCommands(pArena, 1 << 4);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    defer( glEnable(GL_DEPTH_TEST) );
    defer( glEnable(GL_CULL_FACE) );

    s_pShTexMonoBlur->use();
    s_text.bind();
    s_texLiberation.bind(GL_TEXTURE0);

    const math::M4 proj = math::M4Ortho(0, ::ui::WIDTH, ::ui::HEIGHT, 0, -10.0f, 10.0f);

    s_pShTexMonoBlur->setV2("u_texelSize", math::V2From(1.0f/s_texLiberation.m_width, 1.0f/s_texLiberation.m_height));

    /* fps */
    {
        s_pShTexMonoBlur->setV4("u_color", V4From(colors::GREEN, 0.75f));
        s_pShTexMonoBlur->setM4("u_trm", proj * math::M4TranslationFrom({0.0f, 0.0f, -1.0f}));

        s_text.update(s_rastLiberation, &app::g_threadPool.scratchBuffer(), frame::g_sfFpsStatus, true);
        s_text.draw();
    }

    /* info */
    {
        char* pBuff = pArena->zallocV<char>(1 << 9);
        isize n = print::toSpan({pBuff, 1 << 9},
            "F: toggle fullscreen ({})\n"
            "V: toggle VSync ({})\n"
            "R: lock/unlock mouse ({})\n"
            "P: pause/unpause simulation ({})\n"
            "H: draw UI ({})\n"
            "Q/Escape: quit\n"
            ,
            app::windowInst().m_bFullscreen ? "on" : "off",
            app::windowInst().m_swapInterval == 1 ? "on" : "off",
            app::windowInst().m_bPointerRelativeMode ? "locked" : "unlocked",
            control::g_bPauseSimulation ? "paused" : "unpaused",
            control::g_bDrawUI
        );

        StringView sv = {pBuff, n};

        int nSpaces = 0;
        for (auto ch : sv) if (ch == '\n') ++nSpaces;

        s_pShTexMonoBlur->setV4("u_color", V4From(colors::WHITE, 0.75f));
        s_pShTexMonoBlur->setM4("u_trm", proj * math::M4TranslationFrom(
                {0.0f, ::ui::HEIGHT - static_cast<f32>(nSpaces), -1.0f}
            )
        );

        s_text.update(s_rastLiberation, &app::g_threadPool.scratchBuffer(), sv, true);
        s_text.draw();
    }

    drawWidgets(pArena, &vCommands, proj);

    for (auto& command : vCommands)
        command.pfn(command.pArg);
}

} /* namespace render::gl::ui */
