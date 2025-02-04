#include "asset.hh"
#include "bmp.hh"

#include "adt/Map.hh"
#include "adt/file.hh"
#include "adt/OsAllocator.hh"
#include "gltf/gltf.hh"

using namespace adt;

namespace asset
{

Pool<Object, 128> g_assets(INIT);
static Map<String, PoolHnd> s_mapAssets(OsAllocatorGet(), g_assets.getCap());

void
Object::destroy()
{
    LOG_NOTIFY("destroy(): hnd: {}, mappedWith: '{}'\n", g_assets.idx(this), m_sMappedWith);

    s_mapAssets.remove(m_sMappedWith);
    m_arena.freeAll();
    *this = {};
    g_assets.giveBack(this);
}

static Opt<PoolHnd>
loadBMP(const String svPath, String sFile)
{
    bmp::Reader reader {};
    if (!reader.read(sFile))
        return {};

    Object nObj(sFile.getSize() * 1.33);

    Image img = reader.getImage();
    if (img.m_eType == IMAGE_TYPE::RGB)
    {
        Image nImg = img.cloneToARGB(&nObj.m_arena);
        img = nImg;
    }

    nObj.m_uData.img = img;
    nObj.m_eType = TYPE::IMAGE;

    PoolHnd hnd = g_assets.push(nObj);

    return hnd;
}

static Opt<PoolHnd>
loadGLTF(const String svPath, String sFile)
{
    Object nObj(sFile.getSize() * 1.33);

    gltf::Model gltfModel(&nObj.m_arena);
    bool bSucces = gltfModel.read(svPath, sFile);
    if (!bSucces)
    {
        nObj.destroy();
        return {};
    }

    nObj.m_uData.model = gltfModel;
    nObj.m_eType = TYPE::MODEL;

    PoolHnd hnd = g_assets.push(nObj);

    for (auto& image : gltfModel.m_aImages)
    {
        const ssize strSize = 20 + image.uri.getSize();
        char* aBuff = nObj.m_arena.zallocV<char>(strSize);
        ssize n = print::toBuffer(aBuff, strSize - 1, "assets/{}", image.uri);
        Opt<PoolHnd> oHnd = load({aBuff, n});
    }

    return hnd;
}

Opt<PoolHnd>
load(adt::String svPath)
{
    auto found = s_mapAssets.search(svPath);
    if (found)
    {
        LOG_WARN("load(): '{}' is already loaded\n", svPath);
        return found.value();
    }

    Opt<String> osFile = file::load(OsAllocatorGet(), svPath);
    if (!osFile)
        return {};

    String sFile = osFile.value();
    defer( sFile.destroy(OsAllocatorGet()) );

    Opt<PoolHnd> oRet {};

    if (svPath.endsWith(".bmp"))
    {
        oRet = loadBMP(svPath, sFile);
    }
    else if (svPath.endsWith(".gltf"))
    {
        oRet = loadGLTF(svPath, sFile);
    }

    if (oRet)
    {
        auto& obj = g_assets[oRet.value()];
        obj.m_sMappedWith = svPath.clone(&obj.m_arena);
        s_mapAssets.insert(obj.m_sMappedWith, oRet.value());
        LOG_GOOD("load(): hnd: {}, type: '{}', mappedWith: '{}'\n", oRet.value(), obj.m_eType, obj.m_sMappedWith);
    }
    else
    {
        LOG_BAD("load(): failed to load: '{}'\n", svPath);
    }

    return oRet;
}

Object*
search(adt::String svKey, TYPE eType)
{
    auto f = s_mapAssets.search(svKey);

    if (f)
    {
        auto r = &g_assets[f.data().val];
        if (r->m_eType != eType)
        {
            LOG_WARN("sKey: '{}', types don't match, got {}, asked for {}\n", svKey, (int)r->m_eType, (int)eType);
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
searchImage(adt::String svKey)
{
    auto* f = search(svKey, TYPE::IMAGE);
    if (f)
        return &f->m_uData.img;
    else return nullptr;
}

gltf::Model*
searchModel(adt::String svKey)
{
    auto* f = search(svKey, TYPE::MODEL);
    if (f)
        return &f->m_uData.model;
    else return nullptr;
}

} /* namespace asset */
