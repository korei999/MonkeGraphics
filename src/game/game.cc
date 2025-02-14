#include "game.hh"
#include "Entity.hh"

#include "asset.hh"
#include "control.hh"

#include "adt/logs.hh"

using namespace adt;

namespace game
{

struct AssetMapping
{
    String svPath {};
    String svMapTo {};
};

EntityPoolSOA<128> g_aEntites(adt::INIT);

static const String s_aAssetsToLoad[] {
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
};

void
loadStuff()
{
    for (const auto& sPath : s_aAssetsToLoad)
    {
        if (!asset::load(sPath))
            LOG_BAD("failed to load: '{}'\n", sPath);
    }

    {
        auto hTest = g_aEntites.makeDefault();
        auto bind = g_aEntites[hTest];

        auto* pObj = asset::search("assets/BoxAnimated/BoxAnimated.gltf", asset::Object::TYPE::MODEL);
        auto idx = asset::g_aObjects.idx(pObj);
        bind.assetI = idx;
    }

    {
        auto hTest = g_aEntites.makeDefault();
        auto bind = g_aEntites[hTest];

        auto* pObj = asset::search("assets/duck/Duck.gltf", asset::Object::TYPE::MODEL);
        auto idx = asset::g_aObjects.idx(pObj);
        bind.assetI = idx;
    }
}

void
updateState(adt::Arena*)
{
    using namespace control;

    g_camera.updatePos();

    auto what = g_aEntites[{0}];
    what.pos = {std::sinf(frame::g_time) * 3.0f, 0.0f, std::cosf(frame::g_time) * 3.0f};

    /*auto what1 = g_aEntites[{1}];*/
    /*what1.pos = {std::cosf(frame::g_time) * 3.0f, 0.0f, std::sinf(frame::g_time) * 3.0f};*/
}

} /* namespace game */
