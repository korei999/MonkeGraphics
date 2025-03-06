#pragma once

#include "adt/math.hh"
#include "adt/print.hh"

namespace game
{

struct Entity
{
    adt::math::V3 pos {};
    adt::math::Qt rot = adt::math::QtIden();
    adt::math::V3 scale {1.0f, 1.0f, 1.0f};

    adt::math::V3 vel {};

    adt::i16 assetI = -1;
    adt::i16 modelI = -1;

    bool bInvisible = false;
};

struct EntityBind
{
    adt::math::V3& pos;
    adt::math::Qt& rot;
    adt::math::V3& scale;

    adt::math::V3& vel;

    adt::i16& assetI;
    adt::i16& modelI;

    bool& bInvisible;
};

} /* namespace game */

namespace adt::print
{

inline ssize
formatToContext(Context ctx, FormatArgs, const game::EntityBind& x)
{
    ctx.fmt =
        "\n\tpos: [{}]"
        "\n\trot: [{}]"
        "\n\tscale: [{}]"
        "\n\tvel: [{}]"
        "\n\tassetI: {}"
        "\n\tmodelI: {}"
        "\n\tbInvisible: {}";

    ctx.fmtIdx = 0;
    return printArgs(ctx, x.pos, x.rot, x.scale, x.vel, x.assetI, x.modelI, x.bInvisible);
}

} /* namespace adt::print */
