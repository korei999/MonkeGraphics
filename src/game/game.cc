#include "game.hh"
#include "Entity.hh"
#include "Model.hh"

#include "asset.hh"
#include "control.hh"

#include "adt/logs.hh"
#include "adt/PoolSOA.hh"

using namespace adt;

namespace game
{

struct AssetMapping
{
    StringView svPath {};
    StringView svMapTo {};
};

PoolSOA<Entity, EntityBind, MAX_ENTITIES,
    &Entity::pos, &Entity::rot, &Entity::scale,
    &Entity::vel,
    &Entity::assetI, &Entity::shaderI, &Entity::modelI,
    &Entity::bInvisible
> g_poolEntites {};

static const StringView s_aAssetsToLoad[] {
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
    "assets/SimpleSkin/glTF/SimpleSkin.gltf",
    "assets/Fox/Fox.gltf",
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

    /*control::g_camera.m_pos = math::V3{0.0f, 0.0f, -10.0f};*/

    {
        PoolSOAHandle<Entity> hTest = g_poolEntites.make({});
        auto bind = g_poolEntites[hTest];

        if (auto* pObj = asset::search("assets/Fox/Fox.gltf", asset::Object::TYPE::MODEL))
        /*if (auto* pObj = asset::search("assets/BoxAnimated/BoxAnimated.gltf", asset::Object::TYPE::MODEL))*/
        /*if (auto* pObj = asset::search("assets/SimpleSkin/glTF/SimpleSkin.gltf", asset::Object::TYPE::MODEL))*/
        {
            auto idx = asset::g_poolObjects.idx(pObj);
            bind.assetI = idx;

            auto hModel = Model::make(bind.assetI);
            bind.modelI = hModel.i;
        }

        LOG("entity #{}: {}\n", hTest.i, bind);
    }
}

void
updateState(adt::Arena*)
{
    control::g_camera.updatePos();

    // auto what = g_poolEntites[{0}];
    // what.pos = {std::sinf(frame::g_time) * 10.0f, 0.0f, std::cosf(frame::g_time) * 10.0f};
    // what.rot = math::QtAxisAngle(math::V3Norm({1.0f, 1.0f, 1.0f}), -frame::g_time);

    auto what1 = g_poolEntites[{0}];
    what1.pos = {0.0f, 0.0f, 5.0f};
    what1.scale = {0.05f, 0.05f, 0.05f};
    what1.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32 / 2.0f);

    Model::fromI(what1.modelI).m_animationI = 2;

    /*what1.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, frame::g_time);*/

    // for (ssize i = 0; i < g_poolEntites.m_size; ++i)
    // {
    //     if (g_poolEntites.m_arrays.priv.abFree[i])
    //         continue;

    //     EntityBind bind = g_poolEntites[Entity{.i = i}];
    // }
}

} /* namespace game */
