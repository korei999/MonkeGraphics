#pragma once

#include "adt/math.hh"

namespace game
{

struct Entity
{
    adt::StringFixed<128> sfName {};

    adt::math::V3 pos {};
    adt::math::Qt rot = adt::math::QtIden();
    adt::math::V3 scale {1.0f, 1.0f, 1.0f};

    adt::math::V3 vel {};

    adt::i16 assetI = -1;
    adt::i16 modelI = -1;

    bool bNoDraw = false;
};

struct EntityBind
{
    adt::StringFixed<128>& sfName;

    adt::math::V3& pos;
    adt::math::Qt& rot;
    adt::math::V3& scale;

    adt::math::V3& vel;

    adt::i16& assetI;
    adt::i16& modelI;

    bool& bNoDraw;
};

} /* namespace game */

namespace adt::print
{

inline ssize
formatToContext(Context ctx, FormatArgs, const game::EntityBind& x)
{
    ctx.fmt =
        "\n\tname: '{}'"
        "\n\tpos: [{}]"
        "\n\trot: [{}]"
        "\n\tscale: [{}]"
        "\n\tvel: [{}]"
        "\n\tassetI: {}"
        "\n\tmodelI: {}"
        "\n\tbInvisible: {}";

    ctx.fmtIdx = 0;
    return printArgs(ctx, x.sfName, x.pos, x.rot, x.scale, x.vel, x.assetI, x.modelI, x.bNoDraw);
}

} /* namespace adt::print */
