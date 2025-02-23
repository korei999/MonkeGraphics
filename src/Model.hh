#pragma once

#include "adt/Pool.hh"
#include "adt/Map.hh"
#include "adt/math.hh"
#include "adt/Arena.hh"

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
        adt::Vec<adt::i16> vChildren {};

        /* */

        adt::math::M4 getTrm() const
        {
            return adt::math::M4TranslationFrom(translation) *
                adt::math::QtRot(rotation) *
                adt::math::M4ScaleFrom(scale);
        }
    };

    /* */

    adt::Arena m_arena {};
    adt::Map<int, int, adt::hash::dumbFunc> m_mapNodeIToJointI {};
    adt::Vec<Joint> m_vJoints {};
    adt::Vec<adt::math::M4> m_vJointTrms {};
    adt::f64 m_time {};
    adt::f64 m_globalMinTime {};
    adt::f64 m_globalMaxTime {};
    adt::i16 m_modelAssetI {};
    adt::i16 m_rootJointI = -1;

    /* */

    static adt::Pool<Model, 128> s_poolModels;

    /* */

    Model() = default;
    Model(adt::i16 assetModelI);

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

    void loadJoint(adt::i16 gltfNodeI, adt::i16 parentJointI);
    void updateAnimations();
    void update();

private:
    void updateJoint(adt::i16 jointI);
    void updateSkeletalTransofms(adt::math::M4 trm);
    void updateGlobalTransforms(adt::i16 jointI, adt::math::M4 parentTrm);
    void updateJointTransforms();
};
