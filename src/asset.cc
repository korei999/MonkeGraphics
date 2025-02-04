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

static bool
loadBMP(const String sPath, String sFile)
{
    bmp::Reader reader {};
    if (!reader.read(sFile))
        return false;

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
    s_mapAssets.insert(sPath, hnd);

    return true;
}

static bool
loadGLTF(const String sPath, String sFile)
{
    Object nObj(sFile.getSize() * 1.33);

    gltf::Model gltfModel(&nObj.m_arena);
    bool bSucces = gltfModel.read(sPath, sFile);
    if (!bSucces)
        return false;

    nObj.m_uData.model = gltfModel;
    nObj.m_eType = TYPE::MODEL;

    PoolHnd hnd = g_assets.push(nObj);
    s_mapAssets.insert(sPath, hnd);

    return true;
}

bool
load(adt::String sPath)
{
    if (s_mapAssets.search(sPath))
        return true;

    Opt<String> osFile = file::load(OsAllocatorGet(), sPath);
    if (!osFile)
        return false;

    String sFile = osFile.value();
    defer( sFile.destroy(OsAllocatorGet()) );

    if (sPath.endsWith(".bmp"))
    {
        return loadBMP(sPath, sFile);
    }
    else if (sPath.endsWith(".gltf"))
    {
        return loadGLTF(sPath, sFile);
    }
    else
    {
        return false;
    }
}

Object*
search(adt::String sKey, TYPE eType)
{
    auto f = s_mapAssets.search(sKey);

    if (f)
    {
        auto r = &g_assets[f.data().val];
        ADT_ASSERT(r->m_eType == eType, "key: '%.*s': types don't match: %d and %d",
            (int)sKey.getSize(), sKey.data(), (int)r->m_eType, (int)eType
        );
        return r;
    }
    else
    {
        return nullptr;
    }
}

Image*
searchImage(adt::String sKey)
{
    auto* f = search(sKey, TYPE::IMAGE);
    if (f)
        return &f->m_uData.img;
    else return nullptr;
}

gltf::Model*
searchModel(adt::String sKey)
{
    auto* f = search(sKey, TYPE::MODEL);
    if (f)
        return &f->m_uData.model;
    else return nullptr;
}

} /* namespace asset */
