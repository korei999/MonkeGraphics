#include "ui.hh"

#include "app.hh"
#include "control.hh"
#include "game/game.hh"
#include "Model.hh"

#include "adt/logs.hh"

using namespace adt;

namespace ui
{

struct ClickResult
{
    enum class FLAG : u8 { UNHANDLED, HANDLED, GRAB };

    /* */

    Offset off {};
    FLAG eFlag {};
};

static void procCallbacks();

static ClickResult clickArrowList(
    Widget* pWidget,
    Entry* pEntry,
    const f32 px,
    const f32 py,
    const Offset off
);

Pool<Widget, 64> g_poolWidgets;

static bool s_bGrabbed = false;
static Widget* s_pGrabbedWidget {};
static Widget s_dummyWidget {};
static bool s_bPressed = false;

const StringView
Entry::ArrowList::getName() const
{
    if (this->selectedI >= 0 && this->selectedI < this->vEntries.size())
    {
        auto& en = this->vEntries[this->selectedI];
        switch (en.eType)
        {
            case TYPE::TEXT: return en.text.sfName;
            case TYPE::ARROW_LIST: return en.arrowList.getName();
            case TYPE::MENU: return en.menu.sfName;
        }

        ADT_ASSERT(false, "bad path");
        return {};
    }
    else
    {
        return {};
    }
}

void
init()
{
    const auto& win = app::windowInst();

    /* FIXME: grabWidth and grabHeight restrict the clickable area. */
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
                        .pfn = +[](Entry* pSelf, void* p) -> void
                        {
                            ssize enI = reinterpret_cast<ssize>(p);
                            auto enBind = game::g_poolEntities[{static_cast<int>(enI)}];
                            const Model& m = Model::fromI(enBind.modelI);
                            pSelf->menu.selectedI = m.m_animationIUsed;
                        },
                        .pArg = reinterpret_cast<void*>(entityI),
                    },
                    .onClick {
                        .pfn = +[](Entry* pSelf, void* p) -> void
                        {
                            ssize enI = reinterpret_cast<ssize>(p);
                            auto enBind = game::g_poolEntities[{static_cast<int>(enI)}];
                            Model& m = Model::fromI(enBind.modelI);
                            m.m_animationIUsed = pSelf->menu.selectedI;
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
                    .pfn = +[](Entry* pSelf, void* p) -> void
                    {
                        auto& list = pSelf->arrowList;

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
            ssize n = print::toSpan(aBuff, "text #{}", i);
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

        Entry textUnderFirstMenu {
            .text {
                .sfName = "Text under the MenuInMenu",
            },
            .eType = Entry::TYPE::TEXT,
        };

        vEntries.push(&newWidget.arena, textUnderFirstMenu);

        Entry menu {
            .menu {
                .sfName = "Menu0",
                .vEntries = vEntries,
            },
            .eType = Entry::TYPE::MENU,
        };

        newWidget.vEntries.push(&newWidget.arena, menu);

        Entry textUnder {
            .text {
                .sfName = "Text under menus",
            },
            .eType = Entry::TYPE::TEXT,
        };

        newWidget.vEntries.push(&newWidget.arena, textUnder);

        auto h = g_poolWidgets.make(newWidget);
    }

    {
        Widget widget {
            .arena = Arena {SIZE_1K},
            .sfTitle = "ARROW_LIST with text",
            .x = WIDTH - 30.0f,
            .y = 25.0f,
            .border = 0.5f,
            .eFlags = Widget::FLAGS::TITLE | Widget::FLAGS::DRAG,
        };

        Vec<Entry> vTexts {&widget.arena, 6};
        for (ssize i = 0; i < vTexts.cap(); ++i)
        {
            char aBuff[64] {};
            ssize nWritten = print::toSpan(aBuff, "text #{}", i);
            vTexts.push(&widget.arena, {
                .text {StringView{aBuff, nWritten}},
                .eType = Entry::TYPE::TEXT,
            });
        }

        Entry arrowList {
            .arrowList {
                .vEntries = vTexts,
            },
            .eType = Entry::TYPE::ARROW_LIST,
        };

        widget.vEntries.push(&widget.arena, arrowList);
        auto h = g_poolWidgets.make(widget);
    }

    {
        Widget widget {
            .arena = Arena {SIZE_1K},
            .sfTitle = "Menu with arrowList entry",
            .x = WIDTH - 30.0f,
            .y = 30.0f,
            .border = 0.5f,
            .eFlags = Widget::FLAGS::TITLE | Widget::FLAGS::DRAG,
        };

        Vec<Entry> vListText {&widget.arena, 4};
        for (ssize i = 0; i < vListText.cap(); ++i)
        {
            char aBuff[16] {};
            ssize n = print::toSpan(aBuff, "text #{}", i);
            vListText.push(&widget.arena, {
                .text {StringView{aBuff, n}},
                .eType = Entry::TYPE::TEXT,
            });
        }

        vListText.push(&widget.arena, {
            .text = {"Hi"},
            .eType = Entry::TYPE::TEXT,
        });

        Entry arrowList {
            .arrowList {
                .vEntries = vListText,
            },
            .eType = Entry::TYPE::ARROW_LIST,
        };

        Vec<Entry> vList {&widget.arena, 2};
        vList.push(&widget.arena, arrowList);
        vList.push(&widget.arena, {
            .text {"text under arrowList"},
            .eType = Entry::TYPE::TEXT,
        });

        Entry menu {
            .menu {
                .sfName = "Menu",
                .vEntries = vList,
            },
            .eType = Entry::TYPE::MENU,
        };

        widget.vEntries.push(&widget.arena, menu);
        auto h = g_poolWidgets.make(widget);
    }

    procCallbacks();
}

static ClickResult
clickMenu(
    Widget* pWidget,
    Entry* pEntry,
    const f32 px,
    const f32 py,
    const Offset off
)
{
    ADT_ASSERT(pEntry->eType == Entry::TYPE::MENU, " ");

    ClickResult ret {};

    Offset thisOff {0, 0};

    auto& menu = pEntry->menu;

    if (py <= (pWidget->y + off.y + menu.vEntries.size()) &&
        py >= (pWidget->y + off.y)
    )
    {
        LOG_BAD("first if: p: [{}, {}], off: {}\n", px, py, off);

        for (auto& child : menu.vEntries)
        {
            switch (child.eType)
            {
                case Entry::TYPE::TEXT:
                {
                    if (py < (pWidget->y + off.y + thisOff.y + 1) &&
                        py >= (pWidget->y + off.y + thisOff.y) &&
                        px >= pWidget->x + off.x + thisOff.x &&
                        px < pWidget->x + off.x + thisOff.x + child.text.sfName.size()
                    )
                    {
                        menu.selectedI = menu.vEntries.idx(&child);

                        if (menu.onClick.pfn) menu.onClick.pfn(pEntry, menu.onClick.pArg);

                        ret.eFlag = ClickResult::FLAG::HANDLED;
                        goto GOTO_done;
                    }
                }
                break;

                case Entry::TYPE::ARROW_LIST:
                {
                    const StringView svRootName = child.arrowList.getName();

                    if (py < (pWidget->y + off.y + thisOff.y + 1) &&
                        py >= (pWidget->y + off.y + thisOff.y) &&
                        px >= pWidget->x + off.x + thisOff.x &&
                        px < pWidget->x + off.x + thisOff.x + svRootName.size() + 2
                    )
                    {
                        LOG_NOTIFY("MENU -> ARROW_LIST: {}\n", svRootName);

                        ClickResult res = clickArrowList(pWidget, &child, px, py, {off.x + thisOff.x, off.y + thisOff.y});
                        ret.eFlag = ClickResult::FLAG::HANDLED;
                        goto GOTO_done;
                    }
                }
                break;

                case Entry::TYPE::MENU:
                {
                    LOG_NOTIFY("MENU -> MENU\n");
                    thisOff.y += child.menu.vEntries.size() - 1;
                }
                break;
            }

            ++thisOff.y;
        }
    }

GOTO_done:

    ret.off = thisOff;
    return ret;
}

static ClickResult
clickArrowList(Widget* pWidget, Entry* pEntry, const f32 px, const f32 py, const Offset off)
{
    ADT_ASSERT(pEntry, " ");
    ADT_ASSERT(pEntry->eType == Entry::TYPE::ARROW_LIST, " ");

    ClickResult ret {};
    Offset thisOff {0, 0};

    auto& list = pEntry->arrowList;

    if (py < pWidget->y + off.y + 1 &&
        py >= pWidget->y + off.y &&
        px >= pWidget->x + off.x &&
        px < pWidget->x + off.x + list.vEntries[list.selectedI].text.sfName.size() + 2 /* 2 arrows */
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
            ++thisOff.y;
            ClickResult res = clickMenu(
                pWidget, &list.vEntries[list.selectedI], px, py, {thisOff.x + off.x + 2, thisOff.y + off.y}
            );
            thisOff.y += res.off.y;
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
clickWidget(Widget* pWidget, const f32 px, const f32 py, const Offset off)
{
    ADT_ASSERT(pWidget, " ");

    bool bHandled = false;
    ClickResult ret {};
    Offset thisOff {0, 0};

    if (px + pWidget->border >= pWidget->x && px - pWidget->border < pWidget->x + pWidget->priv.grabWidth &&
        py + pWidget->border >= pWidget->y && py - pWidget->border < pWidget->y + pWidget->priv.grabHeight
    )
    {
        if (bool(pWidget->eFlags & Widget::FLAGS::TITLE))
            ++thisOff.y;

        for (Entry& entry : pWidget->vEntries)
        {
            switch (entry.eType)
            {
                case Entry::TYPE::ARROW_LIST:
                {
                    if (py < pWidget->y + (pWidget->priv.grabHeight - off.y) + 1)
                    {
                        s_bPressed = true;

                        ClickResult res = clickArrowList(
                            pWidget, &entry, px, py, {off.x + thisOff.x, off.y + thisOff.y}
                        );

                        bHandled = res.eFlag == ClickResult::FLAG::HANDLED;

                        thisOff.y += res.off.y;
                    }
                }
                break;

                case Entry::TYPE::MENU:
                {
                    s_bPressed = true;

                    ClickResult res = clickMenu( /* +1 y skip menu name */
                        pWidget, &entry, px, py, {off.x + thisOff.x + 2, off.y + thisOff.y + 1}
                    );

                    bHandled = res.eFlag == ClickResult::FLAG::HANDLED;

                    thisOff.y += res.off.y;
                }
                break;

                case Entry::TYPE::TEXT: break;
            }

            if (bHandled) break;
        }

        if (!bHandled && bool(pWidget->eFlags & Widget::FLAGS::DRAG))
        {
            s_pGrabbedWidget = pWidget;
            s_bGrabbed = true;
        }
    }

    if (!bHandled)
    {
        /* Prevent from hover activations, when first click was into nothing. */
        s_bPressed = true;
    }

    ret.off = off;
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
        s_pGrabbedWidget = &s_dummyWidget;
        return;
    }

    if (s_bPressed && !s_bGrabbed) return;

    const f32 widthFactor = 1.0f/(win.m_winWidth * (1.0f/WIDTH));
    const f32 heightFactor = 1.0f/(win.m_winHeight * (1.0f/HEIGHT));

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
        ClickResult res = clickWidget(&widget, px, py, {0, 0});
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
