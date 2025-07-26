#include "game.hh"

#include "Entity.hh"
#include "Model.hh"
#include "asset.hh"
#include "colors.hh"
#include "frame.hh"

#include "adt/logs.hh"

using namespace adt;

namespace game
{

struct AssetMapping
{
    StringView svPath {};
    StringView svMapTo {};
};

VSOAEntity g_vEntities {MAX_ENTITIES};

adt::MapM<StringFixed<128>, isize> g_mapNamesToEntities {MAX_ENTITIES};

/* TODO: should probably be just a separate array of light sources */
isize g_dirLight;
math::V3 g_ambientLight = colors::WHITE * 0.6f;

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
    "assets/whale/whale.CYCLES.gltf",
};

static isize
makeEntity(const StringView svModel, const StringView svName, ENTITY_TYPE eType)
{
    isize handle = g_vEntities.push({});
    game::Entity::Bind bind = g_vEntities[handle];

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

    LOG("entity #{}: {}\n", handle, bind);

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

    /* NOTE: Skybox needs this cube */
    {
        auto hnd = makeEntity("assets/cube/cube.gltf", "Cube", ENTITY_TYPE::REGULAR);
        auto bind = g_vEntities[hnd];

        bind.bNoDraw = true;
    }

    {
        auto hnd = makeEntity("assets/Capo/capo.gltf", "Capo", ENTITY_TYPE::REGULAR);
        auto bind = g_vEntities[hnd];

        auto& model = Model::fromI(bind.modelI);
        model.m_animationUsedI = 0;
    }

    {
        auto hnd = makeEntity("assets/Fox/Fox.gltf", "Fox", ENTITY_TYPE::REGULAR);
        auto bind = g_vEntities[hnd];

        auto& model = Model::fromI(bind.modelI);
        model.m_animationUsedI = 1;
    }

    {
        auto hnd = makeEntity("assets/RecursiveSkeletons/glTF/RecursiveSkeletons.gltf", "RecursiveSkeletons", ENTITY_TYPE::REGULAR);
        auto bind = g_vEntities[hnd];

        auto& model = Model::fromI(bind.modelI);
        model.m_animationUsedI = 0;
    }

    {
        auto hnd = makeEntity("assets/BoxAnimated/BoxAnimated.gltf", "BoxAnimated", ENTITY_TYPE::REGULAR);
        auto bind = g_vEntities[hnd];

        auto& model = Model::fromI(bind.modelI);
        model.m_animationUsedI = 0;
    }

    {
        auto hnd = makeEntity("assets/cube/cube.gltf", "LightCube", ENTITY_TYPE::LIGHT);
        auto bind = g_vEntities[hnd];

        bind.pos = {-10.0f, 8.0f, -8.0f};
        bind.scale = {0.2f, 0.2f, 0.2f};
        bind.color = math::V4From(colors::WHITESMOKE, 1.0f);
        /*bind.bNoDraw = true;*/

        g_dirLight = hnd;
    }

    {
        auto hnd = makeEntity("assets/whale/whale.CYCLES.gltf", "Whale", ENTITY_TYPE::REGULAR);
        auto bind = g_vEntities[hnd];

        bind.pos = {-9.0f, 0.0f, 5.0f};
        bind.scale = {1.00f, 1.00f, 1.00f};
        bind.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);

        auto& model = Model::fromI(bind.modelI);
        model.m_animationUsedI = 1;
    }

    {
        auto entity = g_vEntities[1];
        entity.pos = {-3.0f, 0.0f, 5.0f};
        entity.scale = {1.0f, 1.0f, 1.0f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        /*entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, frame::g_time);*/

        /*Model::fromI(entity.modelI).m_animationIUsed = 0;*/
    }

    {
        auto entity = g_vEntities[2];
        entity.pos = {0.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        /*Model::fromI(entity.modelI).m_animationIUsed = 2;*/
    }

    {
        auto entity = g_vEntities[3];
        entity.pos = {3.0f, 0.0f, 5.0f};
        entity.scale = {0.01f, 0.01f, 0.01f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        /*Model::fromI(entity.modelI).m_animationIUsed = 0;*/
    }

    {
        auto entity = g_vEntities[4];
        entity.pos = {-6.0f, 0.0f, 5.0f};
        entity.scale = {1.00f, 1.00f, 1.00f};
        entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, math::PI32);
        /*Model::fromI(entity.modelI).m_animationIUsed = 0;*/
    }

    {
        // auto entity = g_poolEntities[{6}];
        /*entity.rot = math::QtAxisAngle({0.0f, 1.0f, 0.0f}, frame::g_time);*/
    }

    Arena firstUpdateArena(SIZE_1K);
    defer( firstUpdateArena.freeAll() );
    updateState(&firstUpdateArena);

    for (auto& model : Model::g_poolModels)
        model.updateAnimation(model.m_time + frame::g_frameTime);
}

void
updateState(Arena*)
{
}

isize
searchEntity(StringView svName)
{
    auto res = g_mapNamesToEntities.search(svName);
    return res.valueOr(-1);
}

} /* namespace game */
