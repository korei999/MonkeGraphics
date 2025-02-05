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
    "assets/Sponza/Sponza.gltf",
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
updateState(adt::Arena* pArena)
{
    control::g_camera.m_lastTrm = control::g_camera.procMoveTRM();
}

} /* namespace game */
