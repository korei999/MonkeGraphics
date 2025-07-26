#pragma once

#include "adt/math.hh"
#include "adt/SOA.hh"

namespace game
{

enum class ENTITY_TYPE : adt::u8
{
    REGULAR, LIGHT
};

#define ENTITY_PP_BIND_I(TYPE, NAME) , &Entity::NAME
#define ENTITY_PP_BIND(TUPLE) ENTITY_PP_BIND_I TUPLE
#define ENTITY_FIELDS \
    (adt::StringFixed<128>, sfName),\
    (adt::math::V4, color),\
    (adt::math::V3, pos),\
    (adt::math::Qt, rot),\
    (adt::math::V3, scale),\
    (adt::math::V3, vel),\
    (adt::i16, assetI),\
    (adt::i16, modelI),\
    (ENTITY_TYPE, eType),\
    (bool, bNoDraw)
ADT_SOA_GEN_STRUCT_ZERO(Entity, Bind, ENTITY_FIELDS);
#define ENTITY_TEMPLATE_ARGS Entity, Entity::Bind ADT_PP_FOR_EACH(ENTITY_PP_BIND, ENTITY_FIELDS)

} /* namespace game */

namespace adt::print
{

inline isize
formatToContext(Context ctx, FormatArgs, const game::Entity::Bind& x)
{
    ctx.fmt =
        "\n\tname: '{}'"
        "\n\tcolor: '{}'"
        "\n\tpos: {}"
        "\n\trot: {}"
        "\n\tscale: {}"
        "\n\tvel: {}"
        "\n\tassetI: {}"
        "\n\tmodelI: {}"
        "\n\ttype: {}"
        "\n\tbNoDraw: {}"
    ;

    ctx.fmtIdx = 0;
    return printArgs(ctx, x.sfName, x.color, x.pos, x.rot, x.scale, x.vel, x.assetI, x.modelI, int(x.eType), x.bNoDraw);
}

} /* namespace adt::print */
