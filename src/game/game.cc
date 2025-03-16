#include "game.hh"

#include "Entity.hh"
#include "Model.hh"
#include "asset.hh"
#include "colors.hh"
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

PoolSOA<Entity, Entity::Bind, MAX_ENTITIES,
    &Entity::sfName,
    &Entity::color,
    &Entity::pos, &Entity::rot, &Entity::scale,
    &Entity::vel,
    &Entity::assetI, &Entity::modelI,
    &Entity::eType,
    &Entity::bNoDraw
> g_poolEntities {};

adt::MapManaged<
    StringFixed<128>,
    PoolSOAHandle<Entity>
> g_mapNamesToEntities(StdAllocator::inst(), MAX_ENTITIES);

PoolSOAHandle<Entity> g_dirLight;
math::V3 g_ambientLight = colors::get(colors::WHITE) * 0.6f;

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

static PoolSOAHandle<Entity>
makeEntity(const StringView svModel, const StringView svName, Entity::TYPE eType)
{
    PoolSOAHandle<Entity> handle = g_poolEntities.make({});
    game::Entity::Bind bind = g_poolEntities[handle];

    if (auto* pObj = asset::search(svModel, asset::Object::TYPE::MODEL))
    {
        auto idx = asset::g_poolObjects.idx(pObj);
        bind.assetI = idx;

        auto hModel = Model::make(bind.assetI);
        bind.modelI = hModel.i;
    }

    bind.sfName = svName;
    g_mapNamesToEntities.insert(bind.sfName, handle);

    bind.eType = eType;

    LOG("entity #{}: {}\n", handle.i, bind);

    return handle;
}

void
loadStuff()
{
    for (const auto& svPath : s_aAssetsToLoad)
    {
        if (!asset::load(svPath))
            LOG_BAD("failed to load: '{}'\n", svPath);
    }

    {
        auto hnd = makeEntity("assets/cube/cube.gltf", "Cube", Entity::TYPE::REGULAR);
        g_poolEntities.bindMember<&Entity::bNoDraw>(hnd) = true;
    }

    makeEntity("assets/Capo/capo.gltf", "Capo", Entity::TYPE::REGULAR);
    makeEntity("assets/Fox/Fox.gltf", "Fox", Entity::TYPE::REGULAR);
    makeEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf", "RecursiveSkeletons", Entity::TYPE::REGULAR);
    makeEntity("assets/BoxAnimated/BoxAnimated.gltf", "BoxAnimated", Entity::TYPE::REGULAR);

    {
        auto hnd = makeEntity("assets/cube/cube.gltf", "LightCube", Entity::TYPE::LIGHT);
        auto bind = g_poolEntities[hnd];

        bind.pos = {-10.0f, 8.0f, -8.0f};
        bind.scale = {0.2f, 0.2f, 0.2f};
        bind.color = math::V4From(colors::get(colors::WHITESMOKE), 1.0f);
        bind.bNoDraw = true;

        g_dirLight = hnd;
    }
}

void
updateState(adt::Arena*)
{
    control::g_camera.updatePos();

    {
        auto entity = g_poolEntities[{1}];
        entity.pos = {-3.0f, 0.0f, 5.0f};
        entity.scale = {1.0f, 1.0f, 1.0f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        /*entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, frame::g_time);*/

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
