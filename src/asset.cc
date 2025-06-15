#include "asset.hh"
#include "app.hh"
#include "BMP.hh"

#include "adt/Directory.hh"
#include "adt/Map.hh"
#include "adt/Pool.hh"
#include "adt/StdAllocator.hh"
#include "adt/file.hh"

using namespace adt;

namespace asset
{

Pool<Object, 128> g_poolObjects {INIT};
static MapManaged<StringView, Pool<Object, 128>::Handle> s_mapStringsToObjects(g_poolObjects.cap());

void
Object::destroy()
{
    LOG_NOTIFY("hnd: {}, mappedWith: '{}'\n", g_poolObjects.idx(this), m_sMappedWith);

    s_mapStringsToObjects.tryRemove(m_sMappedWith);
    m_arena.freeAll();
    g_poolObjects.remove(this);

    *this = {};
}

static Pool<Object, 128>::Handle
loadBMP([[maybe_unused]] const StringView svPath, const StringView sFile)
{
    BMP::Reader reader {};
    if (!reader.read(sFile))
        return {};

    Object nObj(sFile.size() * 1.33);

    Image img = reader.getImage();

    img = img.cloneToRGBA(&nObj.m_arena);
    if (app::g_eWindowType != app::WINDOW_TYPE::WAYLAND_SHM)
        img.swapRedBlue();

    nObj.m_uData.img = img;
    nObj.m_eType = Object::TYPE::IMAGE;

    Pool<Object, 128>::Handle hnd = g_poolObjects.insert(nObj);

    return hnd;
}

static Pool<Object, 128>::Handle
loadGLTF(const StringView svPath, const StringView sFile)
{
    Object nObj(SIZE_1M);
    bool bSucces = false;

    json::Parser parser;
    bSucces = parser.parse(StdAllocator::inst(), sFile);
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

    auto hnd = g_poolObjects.insert(nObj);

    for (const auto& image : gltfModel.m_vImages)
    {
        String sPath = file::replacePathEnding(StdAllocator::inst(), svPath, image.sUri);
        defer( sPath.destroy(StdAllocator::inst()) );
        load(sPath);
    }

    return hnd;
}

static Pool<Object, 128>::Handle
loadTTF([[maybe_unused]] const StringView svPath, const StringView sFile)
{
    Object nObj(SIZE_1K * 500);

    ttf::Font font(&nObj.m_arena, sFile);
    if (!font.parse())
    {
        LOG_BAD("failed to load font '{}'\n", svPath);
        nObj.m_arena.freeAll();
        return {};
    }

    nObj.m_uData.font = font;
    nObj.m_eType = Object::TYPE::FONT;

    auto hnd = g_poolObjects.insert(nObj);
    return hnd;
}

Pool<Object, 128>::Handle
loadFile(const StringView svPath)
{
    StdAllocator stdAlloc {};

    String sPathTmp = String(&stdAlloc, svPath);
    defer( sPathTmp.destroy(&stdAlloc) );

    String sFile = file::load(&stdAlloc, sPathTmp.data());
    if (!sFile) return {};

    /* WARNING: must clone sFile contents */
    defer( sFile.destroy(&stdAlloc) );

    Pool<Object, 128>::Handle retHnd {};

    if (svPath.endsWith(".bmp"))
    {
        retHnd = loadBMP(svPath, sFile);
    }
    else if (svPath.endsWith(".gltf"))
    {
        retHnd = loadGLTF(svPath, sFile);
    }
    else if (svPath.endsWith(".ttf"))
    {
        retHnd = loadTTF(svPath, sFile);
    }

    if (retHnd)
    {
        auto& obj = g_poolObjects[retHnd];
        obj.m_sMappedWith = String(&obj.m_arena, svPath);
        [[maybe_unused]] auto mapRes = s_mapStringsToObjects.insert(obj.m_sMappedWith, retHnd);
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

bool
load(const StringView svPath)
{
    StdAllocator stdAlloc {};

    auto found = s_mapStringsToObjects.search(svPath);
    if (found)
    {
        LOG_WARN("'{}' is already loaded\n", svPath);
        return true;
    }

    char* ntsTemp = stdAlloc.zallocV<char>(svPath.size() + 1);
    defer( stdAlloc.free(ntsTemp) );
    strncpy(ntsTemp, svPath.data(), svPath.size());

    switch (file::fileType(ntsTemp))
    {
        case file::TYPE::DIRECTORY:
        {
            Directory dir(ntsTemp);
            defer( dir.close() );

            for (StringView svEntry : dir)
            {
                String s = file::appendDirPath(&stdAlloc, svPath, svEntry);
                defer( s.destroy(&stdAlloc) );

                loadFile(s);
            }
        }
        break;

        case file::TYPE::FILE:
        loadFile(svPath);
        break;

        default:
        LOG_WARN("unhandled filetype\n");
        return false;
    }

    return true;
}

Object*
search(const StringView svKey, Object::TYPE eType)
{
    auto f = s_mapStringsToObjects.search(svKey);

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
        ADT_ASSERT(false, "failed to find asset: '{}'", static_cast<int>(svKey.size()), svKey.data());
        return nullptr;
    }
}

Image*
searchImage(const StringView svKey)
{
    auto* f = search(svKey, Object::TYPE::IMAGE);
    if (f) return &f->m_uData.img;
    else return nullptr;
}

gltf::Model*
searchModel(const StringView svKey)
{
    auto* f = search(svKey, Object::TYPE::MODEL);
    if (f) return &f->m_uData.model;
    else return nullptr;
}

ttf::Font*
searchFont(const adt::StringView svKey)
{
    auto* f = search(svKey, Object::TYPE::FONT);
    if (f) return &f->m_uData.font;
    else return nullptr;
}

} /* namespace asset */
