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
    &Entity::sfName,
    &Entity::pos, &Entity::rot, &Entity::scale,
    &Entity::vel,
    &Entity::assetI, &Entity::modelI,
    &Entity::bNoDraw
> g_poolEntities {};

adt::MapManaged<
    adt::StringFixed<128>,
    adt::PoolSOAHandle<Entity>
> g_mapNamesToEntities(StdAllocator::inst(), MAX_ENTITIES);

static const StringView s_aAssetsToLoad[] {
    "assets/cube/cube.gltf",
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
    "assets/SimpleSkin/glTF/SimpleSkin.gltf",
    "assets/Fox/Fox.gltf",
    "assets/Sphere/sphere.gltf",
    "assets/Capo/capo.gltf",
    "assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf",
    "assets/skybox",
    "assets/LiberationMono-Regular.ttf",
};

void
loadStuff()
{
    for (const auto& svPath : s_aAssetsToLoad)
    {
        if (!asset::load(svPath))
            LOG_BAD("failed to load: '{}'\n", svPath);
    }

    auto addTestEntity = [&](const StringView svModel, const StringView svName) -> void
    {
        PoolSOAHandle<Entity> handle = g_poolEntities.make({});
        game::EntityBind bind = g_poolEntities[handle];

        if (auto* pObj = asset::search(svModel, asset::Object::TYPE::MODEL))
        {
            auto idx = asset::g_poolObjects.idx(pObj);
            bind.assetI = idx;

            auto hModel = Model::make(bind.assetI);
            bind.modelI = hModel.i;

            bind.sfName = svName;

            g_mapNamesToEntities.insert(bind.sfName, handle);
        }

        LOG("entity #{}: {}\n", handle.i, bind);
    };

    addTestEntity("assets/cube/cube.gltf", "Cube");

    addTestEntity("assets/Capo/capo.gltf", "Capo");
    addTestEntity("assets/Fox/Fox.gltf", "Fox");
    addTestEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf", "RecursiveSkeletons");
    addTestEntity("assets/BoxAnimated/BoxAnimated.gltf", "BoxAnimated");

    {
        auto entity = g_poolEntities[{0}];
        entity.bNoDraw = true;
    }
}

void
updateState(adt::Arena*)
{
    control::g_camera.updatePos();

    {
        auto entity = g_poolEntities[{1}];
        entity.pos = {-3.0f, 0.0f, 5.0f};
        entity.scale = {1.00f, 1.00f, 1.00f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{2}];
        entity.pos = {0.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 2;
    }

    {
        auto entity = g_poolEntities[{3}];
        entity.pos = {3.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }

    {
        auto entity = g_poolEntities[{4}];
        entity.pos = {-6.0f, 0.0f, 5.0f};
        entity.scale = {1.00f, 1.00f, 1.00f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        Model::fromI(entity.modelI).m_animationIUsed = 0;
    }
}

[[nodiscard]]
PoolSOAHandle<Entity> searchEntity(StringView svName)
{
    auto res = g_mapNamesToEntities.search(svName);
    return res.valueOr({});
}

} /* namespace game */
