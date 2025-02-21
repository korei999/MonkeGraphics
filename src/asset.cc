#include "asset.hh"
#include "app.hh"
#include "bmp.hh"
#include "gltf/gltf.hh"

#include "adt/Map.hh"
#include "adt/file.hh"
#include "adt/OsAllocator.hh"
#include "adt/Pool.hh"
#include "adt/Opt.hh"


using namespace adt;

namespace asset
{

Pool<Object, 128> g_poolObjects(INIT);
static Map<StringView, PoolHandle<Object>> s_mapStringToObjects(OsAllocatorGet(), g_poolObjects.getCap());

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

    Object nObj(sFile.getSize() * 1.33);

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
        StringView sPath = file::replacePathEnding(OsAllocatorGet(), svPath, image.sUri);
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

    Opt<StringView> osFile = file::load(OsAllocatorGet(), svPath);
    if (!osFile)
        return {};

    /* WARNING: must clone sFile contents */
    StringView sFile = osFile.value();
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
        LOG_GOOD("hnd: {}, type: '{}', mappedWith: '{}', hash: {}\n",
            retHnd, obj.m_eType, obj.m_sMappedWith, mapRes.hash
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

Object*
fromI(adt::i16 handleI, Object::TYPE eType)
{
    auto& ret = g_poolObjects[{handleI}];
    ADT_ASSERT(ret.m_eType == eType, "types don't match");
    return &ret;
}

Image*
fromImageI(adt::i16 handleI)
{
    return &fromI(handleI, Object::TYPE::IMAGE)->m_uData.img;
}

gltf::Model*
fromModelI(adt::i16 handleI)
{
    return &fromI(handleI, Object::TYPE::MODEL)->m_uData.model;
}

} /* namespace asset */
