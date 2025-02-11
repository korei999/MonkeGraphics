#include "adt/logs.hh"

#include "game.hh"
#include "asset.hh"
#include "control.hh"

using namespace adt;

namespace game
{

static const String s_asAssetsToLoad[] {
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
};

void
loadAssets()
{
    for (const auto& sPath : s_asAssetsToLoad)
    {
        if (!asset::load(sPath))
            LOG_BAD("failed to load: '{}'\n", sPath);
    }
}

void
updateState(adt::Arena*)
{
    using namespace control;

    g_camera.updatePos();
}

} /* namespace game */
