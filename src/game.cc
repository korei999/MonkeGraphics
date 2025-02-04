#include "adt/logs.hh"

#include "game.hh"
#include "asset.hh"

using namespace adt;

namespace game
{

static const String s_asAssetPaths[] {
    "assets/cube.gltf",
    "assets/Duck.gltf",
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

} /* namespace game */
