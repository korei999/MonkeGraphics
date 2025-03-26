#pragma once

#include "adt/math.hh"

namespace game
{

struct Entity
{
    enum class TYPE : adt::u8
    {
        REGULAR, LIGHT
    };

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

        Entity::TYPE& eType;

        bool& bNoDraw;
    };

    /* */

    adt::StringFixed<128> sfName {};

    adt::math::V4 color {};

    adt::math::V3 pos {};
    adt::math::Qt rot = adt::math::QtIden();
    adt::math::V3 scale {1.0f, 1.0f, 1.0f};

    adt::math::V3 vel {};

    adt::i16 assetI = -1;
    adt::i16 modelI = -1;

    TYPE eType = TYPE::REGULAR;

    bool bNoDraw = false;
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
        "\n\toutlineColor: [{}]"
        "\n\tbOutline: {}"
        "\n\tbNoDraw: {}"
    ;

    ctx.fmtIdx = 0;
    return printArgs(ctx, x.sfName, x.color, x.pos, x.rot, x.scale, x.vel, x.assetI, x.modelI, x.bNoDraw);
}

} /* namespace adt::print */
