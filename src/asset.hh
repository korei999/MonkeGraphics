#pragma once

#include "Image.hh"
#include "gltf/gltf.hh"

#include "adt/String.hh"
#include "adt/Pool.hh"
#include "adt/Arena.hh"

namespace asset
{

/* TODO: reference system */
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

    void* m_pExtraData {};

    /* */

    Object() = default;
    Object(adt::ssize prealloc) : m_arena(prealloc) {};

    /* */

    void destroy();
};

adt::PoolHandle<Object> load(const adt::StringView svFilePath);
/* may be null */ [[nodiscard]] Object* search(const adt::StringView svKey, Object::TYPE eType);
/* may be null */ [[nodiscard]] Image* searchImage(const adt::StringView svKey);
/* may be null */ [[nodiscard]] gltf::Model* searchModel(const adt::StringView svKey);

[[nodiscard]] Object* fromI(adt::i16 handleI, Object::TYPE eType);
[[nodiscard]] Image* fromImageI(adt::i16 handleI);
[[nodiscard]] gltf::Model* fromModelI(adt::i16 handleI);

extern adt::Pool<Object, 128> g_poolObjects;

} /* namespace asset */

namespace adt::print
{

[[maybe_unused]] static ssize
formatToContext(Context ctx, FormatArgs, const asset::Object::TYPE e)
{
    ctx.fmt = "{}";
    ctx.fmtIdx = 0;
    
    constexpr StringView asMap[] {
        "NONE", "IMAGE", "MODEL"
    };

    return printArgs(ctx, asMap[static_cast<int>(e)]);
}

} /* namespace adt::print */
