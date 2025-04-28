#pragma once

#include "adt/mathDecl.hh"

namespace game
{

enum class ENTITY_TYPE : adt::u8
{
    REGULAR, LIGHT
};

/* FIXME: problems with SOA generator on win32 */
struct Entity
{
    struct Bind
    {
        adt::StringFixed<128>& sfName;
        adt::math::V4& color;
        adt::math::V3& pos;
        adt::math::Qt& rot;
        adt::math::V3& scale;
        adt::math::V3& vel;
        adt::i16& assetI;
        adt::i16& modelI;
        ENTITY_TYPE& eType;
        bool& bNoDraw;
    };

    /* */

    adt::StringFixed<128> sfName;
    adt::math::V4 color;
    adt::math::V3 pos;
    adt::math::Qt rot;
    adt::math::V3 scale;
    adt::math::V3 vel;
    adt::i16 assetI;
    adt::i16 modelI;
    ENTITY_TYPE eType;
    bool bNoDraw;
};

} /* namespace game */

namespace adt::print
{

inline ssize
formatToContext(Context ctx, FormatArgs, const game::Entity::Bind& x)
{
    ctx.fmt =
        "\n\tname: '{}'"
        "\n\tcolor: '{}'"
        "\n\tpos: [{}]"
        "\n\trot: [{}]"
        "\n\tscale: [{}]"
        "\n\tvel: [{}]"
        "\n\tassetI: {}"
        "\n\tmodelI: {}"
        "\n\ttype: {}"
        "\n\tbNoDraw: {}"
    ;

    ctx.fmtIdx = 0;
    return printArgs(ctx, x.sfName, x.color, x.pos, x.rot, x.scale, x.vel, x.assetI, x.modelI, int(x.eType), x.bNoDraw);
}

} /* namespace adt::print */
