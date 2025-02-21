#pragma once

#include "adt/Pool.hh"
#include "adt/Map.hh"
#include "adt/Arena.hh"
#include "adt/math.hh"

struct Model
{
    struct Joint
    {
        adt::math::M4 globalTrm = adt::math::M4Iden();
        adt::math::M4 bindTrm = adt::math::M4Iden();
        adt::math::V3 translation {};
        adt::math::Qt rotation = adt::math::QtIden();
        adt::math::V3 scale {1.0f, 1.0f, 1.0f};
        adt::i16 parentI = -1;
        adt::VecBase<adt::i16> vChildren {};
    };

    /* */

    adt::Arena m_arena {};
    adt::i16 m_modelAssetI {};
    adt::MapBase<int, int, adt::hash::dumbFunc> m_mapNodeIToJointI {};
    adt::VecBase<Joint> m_vJoints {};
    adt::VecBase<adt::math::M4> m_vJointsTrms {};
    adt::f64 m_time {};

    /* */

    static adt::Pool<Model, 128> s_poolModels;

    /* */

    Model() = default;
    Model(adt::i16 modelAssetI);

    /* */

    template<typename ...ARGS>
    static adt::PoolHandle<Model>
    makeHandle(ARGS&&... args)
    {
        return s_poolModels.push(std::forward<ARGS>(args)...);
    }

    static Model&
    fromI(adt::i16 h)
    {
        return s_poolModels[{h}];
    }

    /* */

    void updateGlobalTransforms(adt::i16 jointI, const adt::math::M4& parentTrm);
    void updateAnimations();
    void updateJointTransforms();

private:
    /*adt::math::Qt interpolateQt(*/
};
