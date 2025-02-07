#include "adt/logs.hh"

#include "game.hh"
#include "asset.hh"
#include "control.hh"

using namespace adt;

namespace game
{

static const String s_asAssetPaths[] {
    "assets/cube.gltf",
    "assets/Duck.gltf",
    "assets/backpack/scene.gltf",
    // "assets/Sponza/Sponza.gltf",
    "assets/vampire/vampire.gltf",
};

void
loadAssets()
{
    for (const auto& sPath : s_asAssetPaths)
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
