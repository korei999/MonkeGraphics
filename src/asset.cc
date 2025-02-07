#include "asset.hh"
#include "bmp.hh"

#include "adt/Map.hh"
#include "adt/file.hh"
#include "adt/OsAllocator.hh"
#include "adt/Pool.hh"
#include "gltf/gltf.hh"

using namespace adt;

namespace asset
{

Pool<Object, 128> g_objects(true);
static Map<String, PoolHnd> s_mapStringToObjects(OsAllocatorGet(), g_objects.getCap());

void
Object::destroy()
{
    LOG_NOTIFY("hnd: {}, mappedWith: '{}'\n", g_objects.idx(this), m_sMappedWith);

    s_mapStringToObjects.tryRemove(m_sMappedWith);
    m_arena.freeAll();
    g_objects.giveBack(this);

    *this = {};
}

static Opt<PoolHnd>
loadBMP(const String svPath, const String sFile)
{
    bmp::Reader reader {};
    if (!reader.read(sFile))
        return {};

    Object nObj(sFile.getSize() * 1.33);

    Image img = reader.getImage();

    img = img.cloneToARGB(&nObj.m_arena);

    nObj.m_uData.img = img;
    nObj.m_eType = TYPE::IMAGE;

    PoolHnd hnd = g_objects.push(nObj);

    return hnd;
}

static Opt<PoolHnd>
loadGLTF(const String svPath, const String sFile)
{
    Object nObj(SIZE_1M);
    bool bSucces = false;

    json::Parser parser;
    bSucces = parser.parse(OsAllocatorGet(), sFile);
    defer( parser.destroy() );

    if (!bSucces)
        return {};

    gltf::Model gltfModel;
    bSucces = gltfModel.read(&nObj.m_arena, parser, svPath);
    if (!bSucces)
    {
        nObj.m_arena.freeAll();
        return {};
    }

    nObj.m_uData.model = gltfModel;
    nObj.m_eType = TYPE::MODEL;

    PoolHnd hnd = g_objects.push(nObj);

    for (const auto& image : gltfModel.m_vImages)
    {
        String sPath = file::replacePathEnding(OsAllocatorGet(), svPath, image.sUri);
        defer( sPath.destroy(OsAllocatorGet()) );
        load(sPath);
    }

    return hnd;
}

Opt<PoolHnd>
load(const adt::String svPath)
{
    auto found = s_mapStringToObjects.search(svPath);
    if (found)
    {
        LOG_WARN("'{}' is already loaded\n", svPath);
        return found.value();
    }

    Opt<String> osFile = file::load(OsAllocatorGet(), svPath);
    if (!osFile)
        return {};

    /* WARNING: must clone sFile contents */
    String sFile = osFile.value();
    defer( sFile.destroy(OsAllocatorGet()) );

    Opt<PoolHnd> oRetHnd {};

    if (svPath.endsWith(".bmp"))
    {
        oRetHnd = loadBMP(svPath, sFile);
    }
    else if (svPath.endsWith(".gltf"))
    {
        oRetHnd = loadGLTF(svPath, sFile);
    }

    if (oRetHnd)
    {
        auto& obj = g_objects[oRetHnd.value()];
        obj.m_sMappedWith = svPath.clone(&obj.m_arena);
        [[maybe_unused]] auto mapRes = s_mapStringToObjects.insert(obj.m_sMappedWith, oRetHnd.value());
        LOG_GOOD("hnd: {}, type: '{}', mappedWith: '{}', hash: {}\n",
            oRetHnd.value(), obj.m_eType, obj.m_sMappedWith, mapRes.hash
        );
    }
    else
    {
        LOG_BAD("failed to load: '{}'\n", svPath);
    }

    return oRetHnd;
}

Object*
search(const adt::String svKey, TYPE eType)
{
    auto f = s_mapStringToObjects.search(svKey);

    if (f)
    {
        auto r = &g_objects[f.data().val];
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
searchImage(const adt::String svKey)
{
    auto* f = search(svKey, TYPE::IMAGE);
    if (f)
        return &f->m_uData.img;
    else return nullptr;
}

gltf::Model*
searchModel(const adt::String svKey)
{
    auto* f = search(svKey, TYPE::MODEL);
    if (f)
        return &f->m_uData.model;
    else return nullptr;
}

} /* namespace asset */
