#include "asset.hh"
#include "bmp.hh"

#include "adt/Map.hh"
#include "adt/file.hh"
#include "adt/OsAllocator.hh"

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

    Image img = reader.getImage();
    if (img.m_eType == IMAGE_TYPE::RGB)
    {
        defer( sFile.destroy(OsAllocatorGet()) );

        Image nImg = img.cloneToARGB(OsAllocatorGet());
        img = nImg;
    }

    Object obj {
        .uData {.img = img},
        .eType = TYPE::IMAGE,
    };

    PoolHnd hObj = g_assets.push(obj);
    s_mapAssets.insert(sPath, hObj);

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

    if (sPath.endsWith(".bmp"))
    {
        return loadBMP(sPath, sFile);
    }
    else
    {
        return false;
    }
}

Object*
search(adt::String sKey)
{
    auto f = s_mapAssets.search(sKey);

    if (f)
    {
        return &g_assets[f.data().val];
    }
    else
    {
        return nullptr;
    }
}

} /* namespace asset */
