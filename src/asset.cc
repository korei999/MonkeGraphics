#include "asset.hh"
#include "app.hh"
#include "bmp.hh"
#include "gltf/gltf.hh"

#include "adt/Map.hh"
#include "adt/file.hh"
#include "adt/OsAllocator.hh"
#include "adt/Pool.hh"

using namespace adt;

namespace asset
{

Pool<Object, 128> g_poolObjects(INIT);
static MapManaged<StringView, PoolHandle<Object>> s_mapStringToObjects(OsAllocatorGet(), g_poolObjects.cap());

void
Object::destroy()
{
    LOG_NOTIFY("hnd: {}, mappedWith: '{}'\n", g_poolObjects.idx(this), m_sMappedWith);

    s_mapStringToObjects.tryRemove(m_sMappedWith);
    m_arena.freeAll();
    g_poolObjects.giveBack(this);

    *this = {};
}

static PoolHandle<Object>
loadBMP([[maybe_unused]] const StringView svPath, const StringView sFile)
{
    bmp::Reader reader {};
    if (!reader.read(sFile))
        return {};

    Object nObj(sFile.size() * 1.33);

    Image img = reader.getImage();

    img = img.cloneToRGBA(&nObj.m_arena);
    if (app::g_eWindowType != app::WINDOW_TYPE::WAYLAND_SHM)
        img.swapRedBlue();

    nObj.m_uData.img = img;
    nObj.m_eType = Object::TYPE::IMAGE;

    PoolHandle hnd = g_poolObjects.push(nObj);

    return hnd;
}

static PoolHandle<Object>
loadGLTF(const StringView svPath, const StringView sFile)
{
    Object nObj(SIZE_1M);
    bool bSucces = false;

    json::Parser parser;
    bSucces = parser.parse(OsAllocatorGet(), sFile);
    defer( parser.destroy() );

    if (!bSucces) return {};

    gltf::Model gltfModel;
    bSucces = gltfModel.read(&nObj.m_arena, parser, svPath);
    if (!bSucces)
    {
        nObj.m_arena.freeAll();
        return {};
    }

    nObj.m_uData.model = gltfModel;
    nObj.m_eType = Object::TYPE::MODEL;

    PoolHandle hnd = g_poolObjects.push(nObj);

    for (const auto& image : gltfModel.m_vImages)
    {
        String sPath = file::replacePathEnding(OsAllocatorGet(), svPath, image.sUri);
        defer( sPath.destroy(OsAllocatorGet()) );
        load(sPath);
    }

    return hnd;
}

PoolHandle<Object>
load(const adt::StringView svPath)
{
    auto found = s_mapStringToObjects.search(svPath);
    if (found)
    {
        LOG_WARN("'{}' is already loaded\n", svPath);
        return found.value();
    }

    String sFile = file::load(OsAllocatorGet(), svPath);
    if (!sFile) return {};

    /* WARNING: must clone sFile contents */
    defer( sFile.destroy(OsAllocatorGet()) );

    PoolHandle<Object> retHnd {};

    if (svPath.endsWith(".bmp"))
    {
        retHnd = loadBMP(svPath, sFile);
    }
    else if (svPath.endsWith(".gltf"))
    {
        retHnd = loadGLTF(svPath, sFile);
    }

    if (retHnd)
    {
        auto& obj = g_poolObjects[retHnd];
        obj.m_sMappedWith = String(&obj.m_arena, svPath);
        [[maybe_unused]] auto mapRes = s_mapStringToObjects.insert(obj.m_sMappedWith, retHnd);
        LOG_GOOD("hnd: {}, type: '{}', mappedWith: '{}', hash: {}, len: {}\n",
            retHnd, obj.m_eType, obj.m_sMappedWith, mapRes.hash, obj.m_sMappedWith.size()
        );
    }
    else
    {
        LOG_BAD("failed to load: '{}'\n", svPath);
    }

    return retHnd;
}

Object*
search(const adt::StringView svKey, Object::TYPE eType)
{
    auto f = s_mapStringToObjects.search(svKey);

    if (f)
    {
        auto r = &g_poolObjects[f.data().val];
        if (r->m_eType != eType)
        {
            LOG_WARN("sKey: '{}', types don't match, got {}, asked for {}\n",
                svKey, (int)r->m_eType, (int)eType
            );
            return nullptr;
        }

        return r;
    }
    else
    {
        ADT_ASSERT(false, "failed to find asset: '%.*s'", static_cast<int>(svKey.size()), svKey.data());
        return nullptr;
    }
}

Image*
searchImage(const adt::StringView svKey)
{
    auto* f = search(svKey, Object::TYPE::IMAGE);
    if (f) return &f->m_uData.img;
    else return nullptr;
}

gltf::Model*
searchModel(const adt::StringView svKey)
{
    auto* f = search(svKey, Object::TYPE::MODEL);
    if (f) return &f->m_uData.model;
    else return nullptr;
}

} /* namespace asset */
