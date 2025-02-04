#pragma once

#include "Image.hh"
#include "gltf/gltf.hh"

#include "adt/String.hh"
#include "adt/Pool.hh"
#include "adt/Arena.hh"
#include "adt/Opt.hh"

namespace asset
{

enum TYPE : adt::u8
{
    NONE, IMAGE, MODEL
};

struct Object
{
    adt::Arena m_arena {};
    adt::String m_sMappedWith {};
    union
    {
        Image img;
        gltf::Model model;
    } m_uData {};
    TYPE m_eType {};

    /* */

    Object() = default;
    Object(adt::ssize prealloc) : m_arena(prealloc) {};

    /* */

    void destroy();
};

extern adt::Pool<Object, 128> g_assets;

adt::Opt<adt::PoolHnd> load(adt::String svFilePath);
[[nodiscard]] Object* search(adt::String svKey, TYPE eType); /* may be null */
[[nodiscard]] Image* searchImage(adt::String svKey);
[[nodiscard]] gltf::Model* searchModel(adt::String svKey);

} /* namespace asset */

namespace adt::print
{

[[maybe_unused]] static ssize
formatToContext(Context ctx, FormatArgs, const asset::TYPE e)
{
    ctx.fmt = "{}";
    ctx.fmtIdx = 0;
    
    const String asMap[] {
        "NONE", "IMAGE", "MODEL"
    };

    return printArgs(ctx, asMap[e]);
}

} /* namespace adt::print */
