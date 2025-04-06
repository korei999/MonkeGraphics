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

struct DrawCommand
{
    void (*pfn)(void* pArg) {};
    void* pArg {};
};

static math::IV2 drawArrowList(VecManaged<DrawCommand>* pVCommands, const ::ui::Widget& widget, const ::ui::Entry& entry, const math::M4& proj, int pXOff, int pYOff);

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
        s_text = Text(100);
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
            math::M4TranslationFrom({widget.x + xOff, widget.y + widget.height - 1 - yOff, -1.0f})
        );
        s_pShTexMonoBlur->setV4("u_color", fgColor);

        s_text.update(s_rastLiberation, sv);
        s_text.draw();
    };

    auto* pClDraw = pVCommands->m_pAlloc->alloc<decltype(clDraw)>(clDraw);

    pVCommands->emplace(+[](void* p)
        {
        auto* pCl = static_cast<decltype(clDraw)*>(p);
        pCl->operator()();
        }, pClDraw
    );

    return {static_cast<int>(sv.size()), 1};
}

static math::IV2
drawMenu(
    VecManaged<DrawCommand>* pVCommands,
    const ::ui::Widget& widget,
    const ::ui::Entry& entry,
    const math::M4& proj,
    int xOff,
    int yOff
)
{
    ADT_ASSERT(entry.eType == ::ui::Entry::TYPE::MENU, " ");

    math::IV2 textOff {0, 1};
    for (const ::ui::Entry& child : entry.menu.vEntries)
    {
        const ssize idx = entry.menu.vEntries.idx(&child);

        switch (child.eType)
        {
            case ::ui::Entry::TYPE::TEXT:
            /*textOff += drawText(widget, child, proj, xOff, yOff + textOff.y);*/
            math::V4 col;
            if (idx == entry.menu.selectedI) col = entry.menu.selColor;
            else col = entry.menu.color;

            textOff += drawText(pVCommands, widget, child.text.sfText, col, proj, xOff + 2, yOff + textOff.y);
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            /*drawArrowList(widget, child, proj, &xOff2, &yOff2);*/
            break;

            case ::ui::Entry::TYPE::MENU:
            /*drawMenu(widget, child, proj, &xOff2, &yOff2);*/
            break;
        }
    }

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
    ADT_ASSERT(entry.eType == ::ui::Entry::TYPE::ARROW_LIST, " ");

    auto xyArrow = drawText(pVCommands, widget, "<", entry.arrowList.arrowColor, proj, xOff, yOff);
    xOff += xyArrow.x;

    int maxx = xyArrow.x;

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
                /*auto textOff = drawText(widget, sel, proj, xOff, yOff);*/
                auto textOff = drawText(pVCommands, widget, sel.menu.sfName, sel.menu.color, proj, xOff, yOff);

                xyArrow.x += textOff.x;

                if (maxx <= xyArrow.x) maxx = xyArrow.x;

                auto xyMenu = drawMenu(pVCommands, widget, sel, proj, xOff, yOff);

                if (maxx <= xyMenu.x) maxx = xyMenu.x;
            }
            break;

            case ::ui::Entry::TYPE::ARROW_LIST:
            /*drawArrowList(widget, sel, proj, &xOff, &yOff);*/
            break;
        }
    }

    math::IV2 xyRet = drawText(pVCommands, widget, ">", entry.arrowList.arrowColor, proj, xyArrow.x, yOff);

    return {maxx, xyRet.y};
}

static void
drawWidget(VecManaged<DrawCommand>* pVCommands, const ::ui::Widget& widget, const math::M4& proj)
{
    int yOff = 0;

    int maxx = 0;

    if (bool(widget.eFlags & ::ui::Widget::FLAGS::TITLE))
    {
        math::IV2 xy = drawText(pVCommands, widget, widget.sfTitle,
            V4From(colors::get(colors::WHITE), 0.75f), proj, 0, 0
        );
        yOff += xy.y;

        if (maxx < xy.x) maxx = xy.x;
    }

    for (const auto& entry : widget.vEntries)
    {
        /*s_pShTexMonoBlur->setM4("u_trm", proj **/
        /*    math::M4TranslationFrom({widget.x, widget.y + widget.height - 1 - yOff, 0.0f})*/
        /*);*/

        int xOff = 0;

        switch (entry.eType)
        {
            case ::ui::Entry::TYPE::ARROW_LIST:
            {
                math::IV2 xy = drawArrowList(pVCommands, widget, entry, proj, xOff, yOff);
                if (maxx < xy.x) maxx = xy.x;
            }
            break;

            case ::ui::Entry::TYPE::TEXT:
            // drawText(widget, entry, proj, &xOff, &yOff);
            break;

            case ::ui::Entry::TYPE::MENU:
            // drawMenu(widget, entry, proj, &xOff, &yOff);
            break;
        }
    }

    /* bg rectangle */
    g_pShColor->use();
    g_pShColor->setM4("u_trm", proj *
        math::M4TranslationFrom({widget.x, widget.y, -5.0f}) *
        math::M4ScaleFrom({maxx, widget.height, 0.0f})
    );
    g_pShColor->setV4("u_color", widget.bgColor);
    g_quad.draw();
}

static void
drawWidgets(Arena*, VecManaged<DrawCommand>* pVCommands)
{
    const math::M4 proj = math::M4Ortho(0, ::ui::WIDTH, 0, ::ui::HEIGHT, -10.0f, 10.0f);

    for (const ::ui::Widget& widget : ::ui::g_poolWidgets)
    {
        if (bool(widget.eFlags & ::ui::Widget::FLAGS::NO_DRAW))
            continue;

        drawWidget(pVCommands, widget, proj);
    }
}

void
draw(Arena* pArena)
{
    /* we need to draw text over the rectangle because of blending, so we just buffer up
     * the draw calls, collect all the offsets and draw them after the rectangle. */
    VecManaged<DrawCommand> vCommands(pArena, 1 << 4);

    glDisable(GL_DEPTH_TEST);
    defer( glEnable(GL_DEPTH_TEST) );

    s_pShTexMonoBlur->use();
    s_text.bind();
    s_texLiberation.bind(GL_TEXTURE0);

    const math::M4 proj = math::M4Ortho(0, ::ui::WIDTH, 0, ::ui::HEIGHT, -10.0f, 10.0f);

    s_pShTexMonoBlur->setV4("u_color", V4From(colors::get(colors::WHITE), 0.75f));
    s_pShTexMonoBlur->setV2("u_texelSize", math::V2From(1.0f/s_texLiberation.m_width, 1.0f/s_texLiberation.m_height));

    /* fps */
    {
        s_pShTexMonoBlur->setM4("u_trm", proj * math::M4TranslationFrom({0.0f, ::ui::HEIGHT - 1.0f, -1.0f}));

        s_text.update(s_rastLiberation, frame::g_sfFpsStatus);
        s_text.draw();
    }

    /* info */
    {
        StringView sv =
            "F: toggle fullscreen\n"
            "V: toggle VSync\n"
            "R: lock/unlock mouse\n"
            "Q/Escape: quit";

        int nSpaces = 0;
        for (auto ch : sv) if (ch == '\n') ++nSpaces;

        s_pShTexMonoBlur->setM4("u_trm", proj * math::M4TranslationFrom({0.0f, static_cast<f32>(nSpaces), -1.0f}));

        s_text.update(s_rastLiberation, sv);
        s_text.draw();
    }

    drawWidgets(pArena, &vCommands);

    for (auto& command : vCommands)
        command.pfn(command.pArg);
}

} /* namespace render::gl::ui */
