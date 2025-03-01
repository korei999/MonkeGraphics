#include "game.hh"
#include "Entity.hh"
#include "Model.hh"

#include "asset.hh"
#include "control.hh"

#include "adt/logs.hh"

using namespace adt;

namespace game
{

struct AssetMapping
{
    StringView svPath {};
    StringView svMapTo {};
};

EntityPoolSOA<128> g_poolEntites(INIT);

static const StringView s_aAssetsToLoad[] {
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
    "assets/SimpleSkin/glTF/SimpleSkin.gltf",
    "assets/Fox/Fox.gltf",
    "/home/korei/source/glTF-Sample-Assets/Models/RiggedSimple/glTF/RiggedSimple.gltf",
    "/home/korei/source/glTF-Sample-Assets/Models/RiggedFigure/glTF/RiggedFigure.gltf",
    "assets/Sphere/sphere.gltf",
};

void
loadStuff()
{
    for (const auto& svPath : s_aAssetsToLoad)
    {
        if (!asset::load(svPath))
            LOG_BAD("failed to load: '{}'\n", svPath);
    }

    // {
    //     auto hTest = g_poolEntites.makeDefault();
    //     auto bind = g_poolEntites[hTest];

    //     if (auto* pObj = asset::search("assets/BoxAnimated/BoxAnimated.gltf", asset::Object::TYPE::MODEL))
    //     {
    //         auto idx = asset::g_poolObjects.idx(pObj);
    //         bind.assetI = idx;

    //         auto hModel = Model::makeHandle(bind.assetI);
    //         bind.modelI = hModel.i;
    //     }
    // }

    {
        auto hTest = g_poolEntites.makeDefault();
        auto bind = g_poolEntites[hTest];

        /*if (auto* pObj = asset::search("assets/BoxAnimated/BoxAnimated.gltf", asset::Object::TYPE::MODEL))*/
        /*if (auto* pObj = asset::search("assets/SimpleSkin/glTF/SimpleSkin.gltf", asset::Object::TYPE::MODEL))*/
        /*if (auto* pObj = asset::search("/home/korei/source/glTF-Sample-Assets/Models/RiggedSimple/glTF/RiggedSimple.gltf", asset::Object::TYPE::MODEL))*/
        if (auto* pObj = asset::search("assets/Fox/Fox.gltf", asset::Object::TYPE::MODEL))
        {
            auto idx = asset::g_poolObjects.idx(pObj);
            bind.assetI = idx;

            auto hModel = Model::makeHandle(bind.assetI);
            bind.modelI = hModel.i;
        }
    }
}

void
updateState(adt::Arena*)
{
    using namespace control;

    g_camera.updatePos();

    // auto what = g_poolEntites[{0}];
    // what.pos = {std::sinf(frame::g_time) * 10.0f, 0.0f, std::cosf(frame::g_time) * 10.0f};
    // what.rot = math::QtAxisAngle(math::V3Norm({1.0f, 1.0f, 1.0f}), -frame::g_time);

    auto what1 = g_poolEntites[{0}];
    what1.scale = {0.05f, 0.05f, 0.05f};
    /*what1.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, frame::g_time);*/

    // for (ssize i = 0; i < g_poolEntites.m_size; ++i)
    // {
    //     if (g_poolEntites.m_arrays.priv.abFree[i])
    //         continue;

    //     EntityBind bind = g_poolEntites[Entity{.i = i}];
    // }
}

} /* namespace game */
