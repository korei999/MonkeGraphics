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
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    ::ui::Offset off
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
        s_texLiberation = Texture(s_rastLiberation.m_altas.spanMono(), GL_LINEAR);
        s_text = Text(255);
    }

    s_quad0to1 = Quad(INIT, Quad::TYPE::ZERO_TO_ONE);
}

static ::ui::Offset
drawText(
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const StringView sv,
    const math::V4 fgColor,
    const math::M4& proj,
    ::ui::Offset off
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

        s_text.update(s_rastLiberation, sv, true);
        s_text.draw();
    };

    auto* pClDraw = pVCommands->m_pAlloc->alloc<decltype(clDraw)>(clDraw);

    pVCommands->emplace(+[](void* p)
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
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const bool bDrawName,
    const math::M4& proj,
    ::ui::Offset off
)
{
    ADT_ASSERT(entry.eType == ::ui::Entry::TYPE::MENU, " ");

    auto& menu = entry.menu;

    ::ui::Offset thisOff {0, 0};

    /* arrowList picks up this name, so we don't draw it twice */
    if (bDrawName)
    {
        auto xy = drawText(pVCommands, widget, menu.sfName, menu.color, proj, off);
        thisOff.y += xy.y;
    }

    for (const ::ui::Entry& child : entry.menu.vEntries)
    {
        const ssize idx = menu.vEntries.idx(&child);
        const math::V4 col = [&] {
            if (idx == menu.selectedI) return menu.selColor;
            else return menu.color;
        }();
        ::ui::Offset xy {};

        switch (child.eType)
        {
            case ::ui::Entry::TYPE::TEXT:
            {
                xy = drawText(pVCommands, widget, child.text.sfName, col, proj, {off.x + 2, off.y + thisOff.y});
            }
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            {
                xy = drawArrowList(pVCommands, widget, child, proj, {off.x + 2, off.y + thisOff.y});
            }
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                xy = drawMenu(pVCommands, widget, child, true, proj, {off.x + 2, off.y + thisOff.y});
            }
            break;
        }

        thisOff.x = utils::max(xy.x, thisOff.x);
        thisOff.y += xy.y;
    }

    /* children were drawn with off.x + 2 */
    thisOff.x += 2;

    return thisOff;
}

static ::ui::Offset
drawArrowList(
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    ::ui::Offset off
)
{
    ADT_ASSERT(entry.eType == ::ui::Entry::TYPE::ARROW_LIST, "");

    auto& arrowList = entry.arrowList;

    ::ui::Offset thisOff {0, 0};

    /* '<' should be on the same line */
    auto arrowOff = drawText(pVCommands, widget, "<", entry.arrowList.arrowColor, proj, off);
    thisOff.x += arrowOff.x;

    if (entry.arrowList.vEntries.size() > 0)
    {
        const auto& sel = entry.arrowList.vEntries[entry.arrowList.selectedI];
        switch (sel.eType)
        {
            case ::ui::Entry::TYPE::TEXT:
            {
                auto xy = drawText(pVCommands, widget, sel.text.sfName, sel.text.color, proj, {off.x + 1, off.y + thisOff.y});
                arrowOff.x += xy.x;

                thisOff.x = utils::max(thisOff.x, arrowOff.x);
                thisOff.y += xy.y;
            }
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                /* draw menu name first */
                {
                    auto xy = drawText(pVCommands, widget, sel.menu.sfName, sel.menu.color, proj, {off.x + 1, off.y + thisOff.y});
                    arrowOff.x += xy.x; /* extend arrow line */

                    thisOff.x = utils::max(thisOff.x, arrowOff.x);
                    thisOff.y += xy.y;
                }

                {
                    auto xyMenu = drawMenu(pVCommands, widget, sel, false, proj, {off.x, off.y + thisOff.y});

                    thisOff.x = utils::max(thisOff.x, xyMenu.x);
                    thisOff.y += xyMenu.y;
                }
            }
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            {
            }
            break;
        }
    }

    drawText(pVCommands, widget, ">", entry.arrowList.arrowColor, proj, {off.x + arrowOff.x, off.y});
    thisOff.x = utils::max(thisOff.x, arrowOff.x + 1); /* +1 for the closing arrow */
    /* shouldn't extend y */

    return thisOff;
}

static void
drawWidget(VecManaged<DrawCommand>* pVCommands, ::ui::Widget* pWidget, const math::M4& proj)
{
    ::ui::Offset off {0, 0};
    ::ui::Offset thisOff {0, 0};

    if (bool(pWidget->eFlags & ::ui::Widget::FLAGS::TITLE))
    {
        ::ui::Offset xy = drawText(pVCommands, *pWidget, pWidget->sfTitle,
            V4From(colors::WHITE, 0.75f), proj, off
        );
        thisOff.x = utils::max(thisOff.x, xy.x);
        thisOff.y += xy.y;
    }

    for (const auto& entry : pWidget->vEntries)
    {
        switch (entry.eType)
        {
            case ::ui::Entry::TYPE::ARROW_LIST:
            {
                auto xy = drawArrowList(pVCommands, *pWidget, entry, proj, {off.x, off.y + thisOff.y});
                thisOff.x = utils::max(thisOff.x, xy.x);
                thisOff.y += xy.y;
            }
            break;

            case ::ui::Entry::TYPE::TEXT:
            {
                auto xy = drawText(pVCommands, *pWidget, entry.text.sfName, entry.text.color, proj, {off.x, off.y + thisOff.y});
                thisOff.x = utils::max(thisOff.x, xy.x);
                thisOff.y += xy.y;
            }
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                auto xy = drawMenu(pVCommands, *pWidget, entry, true, proj, {off.x, off.y + thisOff.y});
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
                pWidget->y - pWidget->border/* *uiWidthToUiHeightInv */,
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
drawWidgets(Arena*, VecManaged<DrawCommand>* pVCommands, const math::M4& proj)
{
    for (::ui::Widget& widget : ::ui::g_poolWidgets)
    {
        if (bool(widget.eFlags & ::ui::Widget::FLAGS::NO_DRAW))
            continue;

        drawWidget(pVCommands, &widget, proj);
    }
}

void
draw(Arena* pArena)
{
    /* Save drawText commands in the buffer. Draw them over the rectangle later. */
    VecManaged<DrawCommand> vCommands(pArena, 1 << 4);

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

        s_text.update(s_rastLiberation, frame::g_sfFpsStatus, true);
        s_text.draw();
    }

    /* info */
    {
        Span<char> spBuff = app::gtl_scratch.nextMemZero<char>(1 << 9);
        ssize n = print::toSpan(spBuff,
            "F: toggle fullscreen ({})\n"
            "V: toggle VSync ({})\n"
            "R: lock/unlock mouse ({})\n"
            "P: pause/unpause simulation ({})\n"
            "Q/Escape: quit\n"
            ,
            app::windowInst().m_bFullscreen ? "on" : "off",
            app::windowInst().m_swapInterval == 1 ? "on" : "off",
            app::windowInst().m_bPointerRelativeMode ? "locked" : "unlocked",
            control::g_bPauseSimulation ? "paused" : "unpaused"
        );

        StringView sv = {spBuff.data(), n};

        int nSpaces = 0;
        for (auto ch : sv) if (ch == '\n') ++nSpaces;

        s_pShTexMonoBlur->setV4("u_color", V4From(colors::WHITE, 0.75f));
        s_pShTexMonoBlur->setM4("u_trm", proj * math::M4TranslationFrom(
                {0.0f, ::ui::HEIGHT - static_cast<f32>(nSpaces), -1.0f}
            )
        );

        s_text.update(s_rastLiberation, sv, true);
        s_text.draw();
    }

    drawWidgets(pArena, &vCommands, proj);

    for (auto& command : vCommands)
        command.pfn(command.pArg);
}

} /* namespace render::gl::ui */
