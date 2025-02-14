#pragma once

#include "Image.hh"
#include "gltf/gltf.hh"

#include "adt/String.hh"
#include "adt/Pool.hh"
#include "adt/Arena.hh"
#include "adt/Opt.hh"

namespace asset
{

struct Object
{
    enum class TYPE : adt::u8 { NONE, IMAGE, MODEL };

    /* */

    union
    {
        Image img;
        gltf::Model model;
    } m_uData {};
    TYPE m_eType {};

    adt::Arena m_arena {};
    adt::String m_sMappedWith {};

    void* pExtraData {};

    /* */

    Object() = default;
    Object(adt::ssize prealloc) : m_arena(prealloc) {};

    /* */

    void destroy();
};

adt::Opt<adt::PoolHnd> load(const adt::String svFilePath);
[[nodiscard]] Object* search(const adt::String svKey, Object::TYPE eType); /* may be null */
[[nodiscard]] Image* searchImage(const adt::String svKey);
[[nodiscard]] gltf::Model* searchModel(const adt::String svKey);

extern adt::Pool<Object, 128> g_aObjects;

} /* namespace asset */

namespace adt::print
{

[[maybe_unused]] static ssize
formatToContext(Context ctx, FormatArgs, const asset::Object::TYPE e)
{
    ctx.fmt = "{}";
    ctx.fmtIdx = 0;
    
    const String asMap[] {
        "NONE", "IMAGE", "MODEL"
    };

    return printArgs(ctx, asMap[static_cast<int>(e)]);
}

} /* namespace adt::print */
