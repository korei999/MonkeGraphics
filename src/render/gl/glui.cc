#include "gl.hh"
#include "glui.hh"

#include "control.hh"
#include "app.hh"
#include "Text.hh"
#include "asset.hh"
#include "colors.hh"
#include "frame.hh"
#include "ui.hh"

#include "adt/logs.hh"

using namespace adt;

namespace render::gl::ui
{

struct DrawCommand
{
    void (*pfn)(void* pArg) {};
    void* pArg {};
};

static math::IV2 drawArrowList(
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    int xOff,
    int yOff
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

static math::IV2
drawText(
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const StringView sv,
    const math::V4 fgColor,
    const math::M4& proj,
    int xOff,
    int yOff
)
{
    auto clDraw = [&widget, sv, proj, fgColor, xOff, yOff]
    {
        s_texLiberation.bind(GL_TEXTURE0);
        s_pShTexMonoBlur->use();

        s_pShTexMonoBlur->setM4("u_trm", proj *
            math::M4TranslationFrom({widget.x + xOff, widget.y + yOff, -1.0f})
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

static math::IV2
drawMenu(
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const bool bDrawName,
    const math::M4& proj,
    int xOff,
    int yOff
)
{
    ADT_ASSERT(entry.eType == ::ui::Entry::TYPE::MENU, " ");

    auto& menu = entry.menu;

    math::IV2 textOff {0, 1};
    int yNameOff = 0;

    if (bDrawName)
    {
        drawText(pVCommands, widget, menu.sfName, menu.color, proj, xOff, yOff);
        yNameOff += 2;
    }

    for (const ::ui::Entry& child : entry.menu.vEntries)
    {
        const ssize idx = menu.vEntries.idx(&child);

        switch (child.eType)
        {
            case ::ui::Entry::TYPE::TEXT:
            {
                math::V4 col;
                if (idx == menu.selectedI) col = menu.selColor;
                else col = menu.color;

                textOff += drawText(pVCommands, widget, child.text.sfName, col, proj, xOff + 2, yOff + textOff.y);
            }
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            {
                /*drawArrowList(widget, child, proj, &xOff2, &yOff2);*/
            }
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                /*drawMenu(widget, child, proj, &xOff2, &yOff2);*/
            }
            break;
        }
    }

    textOff.y += yNameOff;
    return textOff;
}

static math::IV2
drawArrowList(
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    int xOff,
    int yOff
)
{
    ADT_ASSERT(entry.eType == ::ui::Entry::TYPE::ARROW_LIST, "");

    auto& arrowList = entry.arrowList;

    auto xyArrow = drawText(pVCommands, widget, "<", entry.arrowList.arrowColor, proj, xOff, yOff);
    xOff += xyArrow.x;

    int maxx = xyArrow.x;
    int maxy = yOff;

    if (entry.arrowList.vEntries.size() > 0)
    {
        const auto& sel = entry.arrowList.vEntries[entry.arrowList.selectedI];
        switch (sel.eType)
        {
            case ::ui::Entry::TYPE::TEXT:
            /*drawText(widget, sel, proj, &xOff, &yOff);*/
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                /* draw menu name first */
                auto textOff = drawText(pVCommands, widget, sel.menu.sfName, sel.menu.color, proj, xOff, yOff);

                xyArrow.x += textOff.x;

                if (maxx <= xyArrow.x) maxx = xyArrow.x;
                maxy += textOff.y;

                auto xyMenu = drawMenu(pVCommands, widget, sel, false, proj, xOff, yOff);

                if (maxx <= xyMenu.x) maxx = xyMenu.x;
                maxy += xyMenu.y;
            }
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            /*drawArrowList(widget, sel, proj, &xOff, &yOff);*/
            break;
        }
    }

    math::IV2 xyRet = drawText(pVCommands, widget, ">", entry.arrowList.arrowColor, proj, xyArrow.x, yOff);
    if (maxx < xyArrow.x + 1) maxx = xyArrow.x + 1;

    return {maxx, maxy};
}

static void
drawWidget(VecManaged<DrawCommand>* pVCommands, ::ui::Widget* pWidget, const math::M4& proj)
{
    int yOff = 0;

    int maxx = 0;
    int maxy = 0;

    if (bool(pWidget->eFlags & ::ui::Widget::FLAGS::TITLE))
    {
        math::IV2 xy = drawText(pVCommands, *pWidget, pWidget->sfTitle,
            V4From(colors::WHITE, 0.75f), proj, 0, 0
        );
        yOff += xy.y;

        if (maxx < xy.x) maxx = xy.x;
        ++maxy;
    }

    for (const auto& entry : pWidget->vEntries)
    {
        int xOff = 0;
        math::IV2 xy {};

        switch (entry.eType)
        {
            case ::ui::Entry::TYPE::ARROW_LIST:
            {
                xy = drawArrowList(pVCommands, *pWidget, entry, proj, xOff, yOff);
            }
            break;

            case ::ui::Entry::TYPE::TEXT:
            // drawText(widget, entry, proj, &xOff, &yOff);
            break;

            case ::ui::Entry::TYPE::MENU:
            {
                xy = drawMenu(pVCommands, *pWidget, entry, true, proj, xOff, yOff);
            }
            break;
        }

        if (maxx < xy.x) maxx = xy.x;
        if (maxy < xy.y) maxy = xy.y;
        yOff = maxy - 1;
    }

    if (pWidget->width == ::ui::Widget::AUTO_SIZE)
        pWidget->grabWidth = maxx;
    else pWidget->grabWidth = pWidget->width;

    if (pWidget->height == ::ui::Widget::AUTO_SIZE)
        pWidget->grabHeight = maxy - 1;
    else pWidget->grabHeight = pWidget->height;

    // constexpr f32 uiWidthToUiHeightInv = 1.0f / (::ui::WIDTH / ::ui::HEIGHT);

    /* bg rectangle */
    if (pWidget->grabHeight > 0 && pWidget->grabWidth > 0)
    {
        g_pShColor->use();
        g_pShColor->setM4("u_trm", proj *
            math::M4TranslationFrom({pWidget->x - pWidget->border, pWidget->y - pWidget->border/* *uiWidthToUiHeightInv */, -5.0f}) *
            math::M4ScaleFrom({pWidget->grabWidth + pWidget->border*2, pWidget->grabHeight + pWidget->border*2 /* *uiWidthToUiHeightInv */, 0.0f})
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
