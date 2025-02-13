#include "game.hh"
#include "Entity.hh"

#include "asset.hh"
#include "control.hh"

#include "adt/logs.hh"

using namespace adt;

namespace game
{

EntityPoolSOA<128> g_aEntites {};

static const String s_asAssetsToLoad[] {
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
};

void
loadStuff()
{
    for (const auto& sPath : s_asAssetsToLoad)
    {
        if (!asset::load(sPath))
            LOG_BAD("failed to load: '{}'\n", sPath);
    }

    auto hTest = g_aEntites.getHandle();
    auto bind = g_aEntites[hTest];

    auto* pObj = asset::search("assets/BoxAnimated/BoxAnimated.gltf", asset::TYPE::MODEL);
    auto idx = asset::g_objects.idx(pObj);
    bind.assetI = idx;
    bind.scale = {1.0f, 1.0f, 1.0f};
    bind.rot = math::QtIden();
    bind.pos = {};
    bind.vel = {};
}

void
updateState(adt::Arena*)
{
    using namespace control;

    g_camera.updatePos();

    auto what = g_aEntites[{0}];

    what.pos = {std::sinf(frame::g_time), 0.0f, std::cosf(frame::g_time)};
}

} /* namespace game */
