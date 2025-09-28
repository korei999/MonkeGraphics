#pragma once

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

template<>
inline isize
format(Context* pCtx, FormatArgs fmtArgs, const game::Entity::Bind& x)
{
    return formatVariadicStacked(pCtx, fmtArgs,
        "\n\tname: '", x.sfName, "'",
        "\n\tcolor: ", x.color,
        "\n\tpos: ", x.pos,
        "\n\trot: ", x.rot,
        "\n\tscale: ", x.scale,
        "\n\tvel: ", x.vel,
        "\n\tassetI: ", x.assetI,
        "\n\tmodelI: ", x.modelI,
        "\n\ttype: ", x.eType,
        "\n\tbNoDraw: ", x.bNoDraw
    );
}

} /* namespace adt::print */
