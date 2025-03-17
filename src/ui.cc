#include "ui.hh"

#include "app.hh"
#include "colors.hh"

using namespace adt;

namespace ui
{

Pool<Rect, 64> g_poolRects;
MapManaged<StringFixed<16>, PoolHandle<Rect>> g_mapStringsToRectHandles(StdAllocator::inst(), g_poolRects.cap());

void
init()
{
    StdAllocator al;
    const auto& win = app::windowInst();

    auto hTest = g_poolRects.make({
        .sfName = "test",
        .sfTitle = "TestTitle",
        .x = WIDTH - 20,
        .y = HEIGHT - 10,
        .width = 20,
        .height = 10,
        .color = math::V4From(colors::get(colors::BLACK), 0.5f),
        .eFlags = Rect::FLAGS::TITLE,
    });

    g_mapStringsToRectHandles.insert(g_poolRects[hTest].sfName, hTest);
}

} /* namespace ui */
