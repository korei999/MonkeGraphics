#include "ui.hh"

#include "app.hh"
#include "colors.hh"
#include "control.hh"
#include "game/game.hh"
#include "Model.hh"

using namespace adt;

namespace ui
{

Pool<Widget, 64> g_poolWidgets;
MapManaged<StringFixed<16>, PoolHandle<Widget>> g_mapStringsToWidgetHandles(StdAllocator::inst(), g_poolWidgets.cap());

void
init()
{
    const auto& win = app::windowInst();

    {
        Widget newWidget {
            .arena = {500},
            .sfName = "EntityList",
            .sfTitle = "EntityList",
            .x = WIDTH - 20.0f,
            .y = HEIGHT - 10.0f,
            .width = 20.0f,
            .height = 10.0f,
            .bgColor = math::V4From(colors::get(colors::BLACK), 0.5f),
            .eFlags = Widget::FLAGS::TITLE | Widget::FLAGS::DRAG,
        };

        Vec<Entry> vEntityEntries(&newWidget.arena);
        for (auto en : game::g_poolEntities)
        {
            const Model& model = Model::fromI(en.modelI);
            Vec<Entry> vAnimations(&newWidget.arena);

            for (const auto& anim : model.m_vAnimations)
            {
                vAnimations.push(&newWidget.arena, {
                    .text {anim.sName},
                    .fgColor = math::V4From(colors::get(colors::WHITESMOKE), 1.0f),
                    .eType = Entry::TYPE::TEXT,
                    });
            }

            Entry entityEntry {
                .menu {.sfName {en.sfName}, .vEntries {vAnimations}},
                .fgColor = math::V4From(colors::get(colors::WHITE), 1.0f),
                .eType = Entry::TYPE::MENU,
            };
            vEntityEntries.push(&newWidget.arena, entityEntry);
        }

        Entry entityList {
            .arrowList {.vEntries = vEntityEntries, .selectedI = 0, .arrowColor = math::V4From(colors::get(colors::CYAN), 1.0f)},
            .fgColor = math::V4From(colors::get(colors::WHITE), 1.0f),
            .eType = Entry::TYPE::ARROW_LIST,
        };

        newWidget.vEntries.push(&newWidget.arena, entityList);

        auto hTest = g_poolWidgets.make(newWidget);
        g_mapStringsToWidgetHandles.insert(g_poolWidgets[hTest].sfName, hTest);
    }
}

void
updateState()
{
    const auto& win = app::windowInst();

    static bool s_bPressed = false;

    if (!control::g_abPressed[BTN_LEFT])
    {
        control::g_mouse.prevAbs = control::g_mouse.abs;
        s_bPressed = false;

        return;
    }

    constexpr f32 invWidth = 1.0f / WIDTH;
    constexpr f32 invHeight = 1.0f / HEIGHT;

    const f32 widthFactor = 1.0f / (win.m_winWidth * invWidth);
    const f32 heightFactor = 1.0f / (win.m_winHeight * invHeight);

    const f32 px = win.m_pointerSurfaceX * widthFactor;
    const f32 py = win.m_pointerSurfaceY * heightFactor;

    const math::V2 delta = control::g_mouse.abs - control::g_mouse.prevAbs;
    control::g_mouse.prevAbs = control::g_mouse.abs;

    const f32 dX = delta.x * widthFactor;
    const f32 dY = delta.y * heightFactor;

    /* TODO: very fast mouse moves breaks the grab */
    for (Widget& widget : g_poolWidgets)
    {
        int yOff = 0;
        bool bHandled = false;

        if (px >= widget.x && px < widget.x + widget.width &&
            py >= widget.y && py < widget.y + widget.height
        )
        {
            if (bool(widget.eFlags & Widget::FLAGS::TITLE))
                ++yOff;

            for (Entry& entry : widget.vEntries)
            {
                if (py >= widget.y + widget.height - yOff - 1 && py < widget.y + widget.height - yOff)
                {
                    bHandled = true;
                    switch (entry.eType)
                    {
                        case Entry::TYPE::ARROW_LIST:
                        {
                            if (!s_bPressed)
                            {
                                s_bPressed = true;
                                ++entry.arrowList.selectedI %= entry.arrowList.vEntries.size();
                            }
                        }
                        break;

                        case Entry::TYPE::MENU:
                        {
                        }
                        break;

                        case Entry::TYPE::TEXT: break;
                    }
                }

                ++yOff;
            }

            /* grab */
            if (!bHandled && bool(widget.eFlags & Widget::FLAGS::DRAG))
            {
                widget.x += dX;
                widget.y += dY;
            }
        }
    }
}

void
destroy()
{
    for (auto& widget : g_poolWidgets)
        widget.arena.freeAll();

    g_mapStringsToWidgetHandles.destroy();
}

} /* namespace ui */
