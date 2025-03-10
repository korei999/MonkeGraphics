#pragma once

#include "adt/math.hh"
#include "adt/print.hh"

namespace game
{

struct Entity
{
    struct Name
    {
        char m_aBuff[128] {};

        /* */

        Name() = default;
        Name(const adt::StringView svName);

        /* */

        operator adt::StringView();

        /* */

        bool operator==(const Name& other) const;
        bool operator==(const adt::StringView sv) const;

        /* */

        static adt::usize hashFunc(const Name& name);
    };

    /* */

    Name name {};

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
    Entity::Name& name;

    adt::math::V3& pos;
    adt::math::Qt& rot;
    adt::math::V3& scale;

    adt::math::V3& vel;

    adt::i16& assetI;
    adt::i16& modelI;

    bool& bNoDraw;
};

inline
Entity::Name::Name(const adt::StringView svName)
{
    strncpy(m_aBuff,
        svName.data(),
        adt::utils::min(svName.size(), static_cast<adt::ssize>(sizeof(m_aBuff) - 1))
    );
}

inline
Entity::Name::operator adt::StringView()
{
    return adt::StringView(m_aBuff);
}

inline bool
Entity::Name::operator==(const Name& other) const
{
    return strncmp(m_aBuff, other.m_aBuff, sizeof(m_aBuff)) == 0;
}

inline bool
Entity::Name::operator==(const adt::StringView sv) const
{
    return adt::StringView(m_aBuff) == sv;
}

inline adt::usize
Entity::Name::hashFunc(const Name& name)
{
    return adt::hash::func(name.m_aBuff);
}

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
    return printArgs(ctx, x.name.m_aBuff, x.pos, x.rot, x.scale, x.vel, x.assetI, x.modelI, x.bNoDraw);
}

} /* namespace adt::print */
