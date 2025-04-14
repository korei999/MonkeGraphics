#include "ui.hh"

#include "app.hh"
#include "control.hh"
#include "game/game.hh"
#include "Model.hh"

using namespace adt;

namespace ui
{

static void procCallbacks();

Pool<Widget, 64> g_poolWidgets;

static bool s_bGrabbed = false;
static Widget* s_pGrabbedWidget {};
static bool s_bPressed = false;

void
init()
{
    const auto& win = app::windowInst();

    /* FIXME: grabWidth and grabHeight are restricting the clickable area. */
    {
        Widget newWidget {
            .arena = Arena(SIZE_1K * 4),
            .sfTitle = "Entity animations",
            .x = WIDTH - 30.0f,
            .y = 0.0f,
            .width = Widget::AUTO_SIZE,
            .height = Widget::AUTO_SIZE,
            .border = 0.5f,
            .bgColor = math::V4From(colors::BLACK, 0.5f),
            .eFlags = Widget::FLAGS::TITLE | Widget::FLAGS::DRAG,
        };

        Vec<Entry> vEntityEntries(&newWidget.arena);
        ssize entityI = 0;
        for (auto entity : game::g_poolEntities)
        {
            defer( ++entityI );

            if (entity.bNoDraw) continue;

            Model& model = Model::fromI(entity.modelI);
            Vec<Entry> vAnimations(&newWidget.arena);

            for (const auto& anim : model.m_vAnimations)
            {
                vAnimations.push(&newWidget.arena, {
                    .text {.sfName = anim.sName, .color = math::V4From(colors::WHITESMOKE, 1.0f)},
                    .eType = Entry::TYPE::TEXT,
                });
            }

            vAnimations.push(&newWidget.arena, {
                .text = {.sfName = "off", .color = math::V4From(colors::WHITESMOKE, 1.0f)},
                .eType = Entry::TYPE::TEXT,
            });

            Entry entityEntry {
                .menu {
                    .sfName {entity.sfName},
                    .vEntries {vAnimations},
                    .selColor = math::V4From(colors::GREEN, 1.0f),
                    .color = math::V4From(colors::WHITESMOKE, 1.0f),
                    .selectedI = 0,
                    .onUpdate {
                        .pfn = +[](Entry* self, void* p) -> void
                        {
                            ssize enI = reinterpret_cast<ssize>(p);
                            auto enBind = game::g_poolEntities[{static_cast<int>(enI)}];
                            const Model& m = Model::fromI(enBind.modelI);
                            self->menu.selectedI = m.m_animationIUsed;
                        },
                        .pArg = reinterpret_cast<void*>(entityI),
                    },
                    .onClick {
                        .pfn = +[](Entry* self, void* p) -> void
                        {
                            ssize enI = reinterpret_cast<ssize>(p);
                            auto enBind = game::g_poolEntities[{static_cast<int>(enI)}];
                            Model& m = Model::fromI(enBind.modelI);
                            m.m_animationIUsed = self->menu.selectedI;
                        },
                        .pArg = reinterpret_cast<void*>(entityI),
                    },
                },
                .eType = Entry::TYPE::MENU,
            };
            vEntityEntries.push(&newWidget.arena, entityEntry);
        }

        Entry entityList {
            .arrowList {
                .vEntries = vEntityEntries,
                .selectedI = 0,
                .prevSelectedI = 0,
                .color = math::V4From(colors::WHITE, 1.0f),
                .arrowColor = math::V4From(colors::CYAN, 1.0f),
                .onUpdate {
                    .pfn = +[](Entry* self, void* p) -> void
                    {
                        auto& list = self->arrowList;

                        auto set = [&]<bool B_SET>(ssize selI) -> void
                        {
                            if (selI >= 0 && selI < list.vEntries.size())
                            {
                                auto& entry = list.vEntries[selI];
                                ADT_ASSERT(entry.eType == Entry::TYPE::MENU, " ");

                                auto& menu = entry.menu;
                                ssize enI = reinterpret_cast<ssize>(menu.onClick.pArg);
                                auto enBind = game::g_poolEntities[{static_cast<int>(enI)}];
                                Model& model = Model::fromI(enBind.modelI);

                                if constexpr (B_SET)
                                    model.m_oOutlineColor = math::V4From(colors::GAINSBORO, 1.0f);
                                else model.m_oOutlineColor = {};
                            }
                        };

                        if (list.prevSelectedI != list.selectedI)
                            set.operator()<false>(list.prevSelectedI);

                        set.operator()<true>(list.selectedI);
                    },
                    .pArg {},
                },
            },
            .eType = Entry::TYPE::ARROW_LIST,
        };

        newWidget.vEntries.push(&newWidget.arena, entityList);

        auto hTest = g_poolWidgets.make(newWidget);
    }

    {
        Widget newWidget {
            .arena = Arena{SIZE_1K},
            .sfTitle = "~TEST~ (menu entry)",
            .x = WIDTH - 30.0f,
            .y = 10.0f,
            .width = Widget::AUTO_SIZE,
            .height = Widget::AUTO_SIZE,
            .border = 0.5f,
            .bgColor = math::V4From(colors::BLACK, 0.5f),
            .eFlags = Widget::FLAGS::TITLE | Widget::FLAGS::DRAG,
        };

        Vec<Entry> vEntries(&newWidget.arena, 3);
        for (ssize i = 0; i < 3; ++i)
        {
            char aBuff[16] {};
            ssize n = print::toSpan(aBuff, "Test #{}", i);
            vEntries.push(&newWidget.arena, {
                    .text {.sfName = StringView{aBuff, n}},
                    .eType = Entry::TYPE::TEXT,
                }
            );
        }

        {
            Vec<Entry> vMenuEntries {&newWidget.arena, 2};

            vMenuEntries.push(&newWidget.arena, {
                .text {"menuText0"},
                .eType = Entry::TYPE::TEXT,
            });

            vMenuEntries.push(&newWidget.arena, {
                .text {"menuText1"},
                .eType = Entry::TYPE::TEXT,
            });

            Entry menuInMenu {
                .menu {
                    .sfName = "MenuInMenu",
                    .vEntries = vMenuEntries
                },
                .eType = Entry::TYPE::MENU,
            };

            vEntries.push(&newWidget.arena, menuInMenu);
        }

        vEntries.push(&newWidget.arena, {
                .text {.sfName = "text under menu"},
                .eType = Entry::TYPE::TEXT,
            }
        );

        Entry menu {
            .menu {
                .sfName = "What",
                .vEntries = vEntries,
            },
            .eType = Entry::TYPE::MENU,
        };

        newWidget.vEntries.push(&newWidget.arena, menu);

        auto h = g_poolWidgets.make(newWidget);
    }

    procCallbacks();
}

struct ClickResult
{
    enum class FLAG : u8 { UNHANDLED, HANDLED, GRAB };

    /* */

    int xOff {};
    int yOff {};
    FLAG eFlag {};
};

static ClickResult
clickMenu(Widget* pWidget, Entry* pParentEntry, Entry* pEntry, const f32 px, const f32 py, int xOff, int yOff)
{
    ADT_ASSERT(pEntry->eType == Entry::TYPE::MENU, " ");

    ClickResult ret {};

    auto& menu = pEntry->menu;

    if (py <= (pWidget->y + yOff + menu.vEntries.size()) &&
        py >= (pWidget->y + yOff)
    )
    {
        for (auto& child : menu.vEntries)
        {
            if (py < (pWidget->y + yOff + 1) && py >= (pWidget->y + yOff) &&
                px >= pWidget->x + xOff && px < pWidget->x + xOff + child.text.sfName.size()
            )
            {
                menu.selectedI = menu.vEntries.idx(&child);

                if (menu.onClick.pfn) menu.onClick.pfn(pEntry, menu.onClick.pArg);
                ret.eFlag = ClickResult::FLAG::HANDLED;

                break;
            }

            ++yOff;
        }
    }

    ret.xOff = xOff;
    ret.yOff = yOff;

    return ret;
}

static ClickResult
clickArrowList(Widget* pWidget, Entry* pEntry, const f32 px, const f32 py, int xOff, int yOff)
{
    ADT_ASSERT(pEntry, " ");
    ADT_ASSERT(pEntry->eType == Entry::TYPE::ARROW_LIST, " ");

    ClickResult ret {};

    auto& list = pEntry->arrowList;

    if (py < pWidget->y + yOff + 1 &&
        py >= pWidget->y + yOff &&
        px >= pWidget->x + xOff &&
        px < pWidget->x + xOff + list.vEntries[list.selectedI].text.sfName.size() + 2 /* 2 arrows */
    )
    {
        list.prevSelectedI = list.selectedI;

        if (control::g_abPressed[BTN_LEFT])
            list.selectedI = utils::cycleForward(list.selectedI, list.vEntries.size());
        else if (control::g_abPressed[BTN_RIGHT])
            list.selectedI = utils::cycleBackward(list.selectedI, list.vEntries.size());

        ret.eFlag = ClickResult::FLAG::HANDLED;

        return ret;
    }

    switch (pEntry->arrowList.vEntries[pEntry->arrowList.selectedI].eType)
    {
        case Entry::TYPE::MENU:
        {
            ++yOff;
            ClickResult res = clickMenu(
                pWidget, pEntry, &list.vEntries[list.selectedI], px, py, xOff + 2, yOff
            );
            ret = res;
        }
        break;

        case Entry::TYPE::ARROW_LIST:
        {
        }
        break;

        case Entry::TYPE::TEXT:
        {
        }
        break;
    }

    return ret;
}

static ClickResult
clickWidget(Widget* pWidget, const f32 px, const f32 py, int xOff, int yOff)
{
    ADT_ASSERT(pWidget, " ");

    Widget& widget = *pWidget;
    ClickResult ret {};
    bool bHandled = false;

    if (px + widget.border >= widget.x && px - widget.border < widget.x + widget.grabWidth &&
        py + widget.border >= widget.y && py - widget.border < widget.y + widget.grabHeight
    )
    {
        if (bool(widget.eFlags & Widget::FLAGS::TITLE))
            ++yOff;

        for (Entry& entry : widget.vEntries)
        {
            switch (entry.eType)
            {
                case Entry::TYPE::ARROW_LIST:
                {
                    if (py < widget.y + widget.grabHeight - yOff + 1)
                    {
                        s_bPressed = true;

                        ClickResult res = clickArrowList(pWidget, &entry, px, py, xOff, yOff);
                        if (res.eFlag == ClickResult::FLAG::HANDLED)
                            bHandled = true;

                        /*xOff = res.xOff;*/
                        /*yOff = res.yOff;*/
                        yOff = utils::max(yOff, res.yOff);
                    }
                }
                break;

                case Entry::TYPE::MENU:
                {
                }
                break;

                case Entry::TYPE::TEXT: break;
            }

            if (bHandled) break;

            ++yOff;
        }

        if (!bHandled && bool(widget.eFlags & Widget::FLAGS::DRAG))
        {
            s_pGrabbedWidget = &widget;
            s_bGrabbed = true;
        }
    }

    ret.xOff = xOff;
    ret.yOff = yOff;
    return ret;
}

static void
entryCallback(Entry* pEntry)
{
    switch (pEntry->eType)
    {
        case Entry::TYPE::ARROW_LIST:
        {
            auto& list = pEntry->arrowList;
            for (auto& entry : list.vEntries) entryCallback(&entry);

            if (list.onUpdate.pfn) list.onUpdate.pfn(pEntry, list.onUpdate.pArg);
        }
        break;

        case Entry::TYPE::MENU:
        {
            auto& menu = pEntry->menu;
            if (menu.onUpdate.pfn) menu.onUpdate.pfn(pEntry, menu.onUpdate.pArg);

            for (auto& child : menu.vEntries) entryCallback(&child);
        }
        break;

        case Entry::TYPE::TEXT: break;
    }
}

static void
procCallbacks()
{
    for (Widget& widget : g_poolWidgets)
    {
        for (auto& entry : widget.vEntries)
            entryCallback(&entry);
    }
}

void
updateState()
{
    procCallbacks();

    const auto& win = app::windowInst();
    auto& mouse = control::g_mouse;

    if (win.m_bPointerRelativeMode) return;

    mouse.abs = math::V2{win.m_pointerSurfaceX, win.m_pointerSurfaceY};

    const math::V2 delta = mouse.abs - mouse.prevAbs;
    mouse.prevAbs = mouse.abs;

    if (!control::g_abPressed[BTN_LEFT] && !control::g_abPressed[BTN_RIGHT])
    {
        s_bPressed = false;
        s_bGrabbed = false;
        static Widget s_dummyWidget {};
        s_pGrabbedWidget = &s_dummyWidget;
        return;
    }

    if (s_bPressed && !s_bGrabbed) return;

    const f32 widthFactor = 1.0f / (win.m_winWidth * (1.0f/WIDTH));
    const f32 heightFactor = 1.0f / (win.m_winHeight * (1.0f/HEIGHT));

    const f32 px = mouse.abs.x * widthFactor;
    const f32 py = mouse.abs.y * heightFactor;

    const f32 dx = delta.x * widthFactor;
    const f32 dy = delta.y * heightFactor;

    if (s_bGrabbed)
    {
        s_pGrabbedWidget->x += dx;
        s_pGrabbedWidget->y += dy;
        return;
    }

    /* TODO: rework into a rooted tree. */
    for (Widget& widget : g_poolWidgets)
    {
        ClickResult res = clickWidget(&widget, px, py, 0, 0);
        if (res.eFlag == ClickResult::FLAG::GRAB)
            break;
    }
}

void
destroy()
{
    for (auto& widget : g_poolWidgets)
        widget.arena.freeAll();
}

} /* namespace ui */
