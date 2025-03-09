#include "game.hh"

#include "Entity.hh"
#include "Model.hh"
#include "asset.hh"
#include "control.hh"

#include "adt/PoolSOA.hh"
#include "adt/logs.hh"

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
    &Entity::assetI, &Entity::modelI,
    &Entity::bNoDraw
> g_poolEntities {};

static const StringView s_aAssetsToLoad[] {
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
    "assets/SimpleSkin/glTF/SimpleSkin.gltf",
    "assets/Fox/Fox.gltf",
    "assets/Sphere/sphere.gltf",
    "assets/Capo/capo.gltf",
    "assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf",
    "assets/skybox",
};

void
loadStuff()
{
    for (const auto& svPath : s_aAssetsToLoad)
    {
        if (!asset::load(svPath))
            LOG_BAD("failed to load: '{}'\n", svPath);
    }

    auto addTestEntity = [&](const StringView svModel) -> void
    {
        PoolSOAHandle<Entity> hTest = g_poolEntities.make({});
        game::EntityBind bind = g_poolEntities[hTest];

        if (auto* pObj = asset::search(svModel, asset::Object::TYPE::MODEL))
        {
            auto idx = asset::g_poolObjects.idx(pObj);
            bind.assetI = idx;

            auto hModel = Model::make(bind.assetI);
            bind.modelI = hModel.i;
        }

        LOG("entity #{}: {}\n", hTest.i, bind);
    };

    addTestEntity("assets/Capo/capo.gltf");
    addTestEntity("assets/Fox/Fox.gltf");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");

    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf");

    addTestEntity("assets/BoxAnimated/BoxAnimated.gltf");
}

void
updateState(adt::Arena*)
{
    control::g_camera.updatePos();

    {
        auto entity = g_poolEntities[{0}];
        entity.pos = {-3.0f, 0.0f, 5.0f};
        entity.scale = {1.00f, 1.00f, 1.00f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{1}];
        entity.pos = {0.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 2;
    }

    {
        auto entity = g_poolEntities[{2}];
        entity.pos = {3.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{3}];
        entity.pos = {6.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{4}];
        entity.pos = {9.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{5}];
        entity.pos = {12.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{6}];
        entity.pos = {15.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{7}];
        entity.pos = {18.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{8}];
        entity.pos = {21.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{9}];
        entity.pos = {24.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{10}];
        entity.pos = {-6.0f, 0.0f, 5.0f};
        entity.scale = {1.00f, 1.00f, 1.00f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }
}

} /* namespace game */
