#pragma once

#include "adt/Arr.hh"
#include "adt/math.hh"

#define ENTITY_POOL_SOA_RANGE_CHECK ADT_ASSERT(h.i >= 0 && h.i < CAP, "out of range: i: %d, CAP: %d", h.i, CAP)

namespace game
{

 /* idx in the pool */
struct Entity
{
    int i {};

    explicit operator int() const { return i; }
};

struct EntityBind
{
    adt::math::V3& pos;
    adt::math::Qt& rot;
    adt::math::V3& scale;

    adt::math::V3& vel;

    adt::u16& assetI;
    adt::u16& shaderI;

    bool& bDead;
};

template<int CAP> 
struct EntityPoolSOA
{
    struct
    {
        adt::math::V3 aPos[CAP] {};
        adt::math::Qt aRot[CAP] {};
        adt::math::V3 aScale[CAP] {};

        adt::math::V3 aVel[CAP] {};

        adt::u16 aAssetI[CAP] {};
        adt::u16 aShaderI[CAP] {};

        bool abDead[CAP] {};

        struct
        {
            bool abFree[CAP] {};
        } priv {};
    } m_arrays {};

    adt::Arr<Entity, CAP> m_aFreeHandles {};
    int m_size {};

    /* */

    EntityPoolSOA() = default;

    /* */

    EntityBind operator[](Entity h) { return bind(h); }
    const EntityBind operator[](Entity h) const { return bind(h); }

    [[nodiscard]] Entity getHandle();
    void giveBack(Entity h);

private:
    EntityBind bind(Entity h);
};

template<int CAP> 
inline Entity
EntityPoolSOA<CAP>::getHandle()
{
    Entity res {};

    if (!m_aFreeHandles.empty())
        res = *m_aFreeHandles.pop();
    else res.i = m_size++;

    m_arrays.priv.abFree[res.i] = false;
    return res;
}

template<int CAP> 
inline void
EntityPoolSOA<CAP>::giveBack(Entity h)
{
    ADT_ASSERT(m_arrays.abFree[h.i] != true, "handle: %lld is already free", h.i);

    m_arrays.priv.abFree[h.i] = true;
    m_aFreeHandles.push(h);
}

template<int CAP> 
inline EntityBind
EntityPoolSOA<CAP>::bind(Entity h)
{
    ENTITY_POOL_SOA_RANGE_CHECK;
    return {
        .pos = m_arrays.aPos[h.i],
        .rot = m_arrays.aRot[h.i],
        .scale = m_arrays.aScale[h.i],

        .vel = m_arrays.aVel[h.i],

        .assetI = m_arrays.aAssetI[h.i],
        .shaderI = m_arrays.aShaderI[h.i],

        .bDead = m_arrays.abDead[h.i],
    };
}

} /* namespace game */

#undef ENTITY_POOL_SOA_RANGE_CHECK
