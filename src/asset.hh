#pragma once

#include "Image.hh"
#include "gltf/Model.hh"
#include "ttf/Font.hh"

#include "adt/Pool.hh"
#include "adt/Arena.hh"

namespace asset
{

struct Object
{
    enum class TYPE : adt::u8 { NONE, IMAGE, MODEL, FONT };

    /* */

    union
    {
        Image img;
        gltf::Model model;
        struct {
            adt::String sFontFile;
            ttf::Font ttf;
        } font;
    } m_uData {};
    TYPE m_eType {};

    adt::Arena m_arena {};
    adt::String m_sMappedWith {};
    // TODO: adt::Vec<int> m_vObservers {}; /* array of pool handles that refer to this object */

    void* m_pExtraData {};

    /* */

    Object() = default;
    Object(adt::isize prealloc) : m_arena(prealloc) {};

    /* */

    void destroy();
};

bool load(const adt::StringView svFilePath);
adt::Pool<Object, 128>::Handle loadFile(const adt::StringView svPath);
/* may be null */ [[nodiscard]] Object* search(const adt::StringView svKey, Object::TYPE eType);
/* may be null */ [[nodiscard]] Image* searchImage(const adt::StringView svKey);
/* may be null */ [[nodiscard]] gltf::Model* searchModel(const adt::StringView svKey);
/* may be null */ [[nodiscard]] ttf::Font* searchFont(const adt::StringView svKey);

extern adt::Pool<Object, 128> g_poolObjects;

[[nodiscard]] inline Object*
fromI(adt::i16 handleI, [[maybe_unused]] Object::TYPE eType)
{
    auto& ret = g_poolObjects[{handleI}];
    ADT_ASSERT(ret.m_eType == eType, "types don't match");
    return &ret;
}

[[nodiscard]] inline Image*
fromImageI(adt::i16 handleI)
{
    return &fromI(handleI, Object::TYPE::IMAGE)->m_uData.img;
}

[[nodiscard]] inline gltf::Model*
fromModelI(adt::i16 handleI)
{
    return &fromI(handleI, Object::TYPE::MODEL)->m_uData.model;
}

[[nodiscard]] inline ttf::Font*
fromFontI(adt::i16 handleI)
{
    return &fromI(handleI, Object::TYPE::FONT)->m_uData.font.ttf;
}

} /* namespace asset */

namespace adt::print
{

[[maybe_unused]] static isize
formatToContext(Context ctx, FormatArgs, const asset::Object::TYPE e)
{
    ctx.fmt = "{}";
    ctx.fmtIdx = 0;

    constexpr StringView asMap[] {
        "NONE", "IMAGE", "MODEL", "FONT"
    };

    ADT_ASSERT(static_cast<int>(e) < utils::size(asMap), " ");

    return printArgs(ctx, asMap[static_cast<int>(e)]);
}

} /* namespace adt::print */
