#pragma once

#include "gltf/gltf.hh"

#include "adt/Pool.hh"
#include "adt/Arena.hh"

/* Model holds the same order of nodes as the gltf::Model that it refers to */
struct Model
{
    static constexpr int MAX_JOINTS = 128;

    struct Skin
    {
        adt::Vec<adt::math::M4> vJointMatrices {};
    };

    struct Node
    {
        enum class TRANSFORMATION_TYPE : adt::u8 { NONE, MATRIX, ANIMATION };

        /* */

        adt::math::M4 finalTransform = adt::math::M4Iden();

        union Transformation
        {
            adt::math::M4 matrix;
            struct
            {
                adt::math::V3 tra;
                adt::math::Qt rot;
                adt::math::V3 sca;
            };
        } uTransform {};
        TRANSFORMATION_TYPE eType = TRANSFORMATION_TYPE::MATRIX;
    };

    /* */

    adt::Arena m_arena {};

    adt::Vec<Node> m_vNodes {};
    adt::Vec<Skin> m_vSkins {};
    adt::Vec<int> m_vSkinnedNodes {};

    adt::f64 m_time {};
    adt::f64 m_globalMinTime {};
    adt::f64 m_globalMaxTime {};
    adt::i16 m_modelAssetI {};

    int m_animationIUsed = -1;

    /* */

    static adt::Pool<Model, 128> s_poolModels;

    /* */

    Model() = default;
    Model(adt::i16 assetModelI);

    /* */

    template<typename ...ARGS>
    static adt::PoolHandle<Model>
    make(ARGS&&... args)
    {
        return s_poolModels.push(std::forward<ARGS>(args)...);
    }

    static Model&
    fromI(adt::i16 h)
    {
        return s_poolModels[{h}];
    }

    /* */

    gltf::Model& gltfModel() const;
    gltf::Node& gltfNode(const Node& node) const;

    void updateAnimation(int animationI);
    void updateAnimation() { updateAnimation(m_animationIUsed); }
    void updateSkins();

private:
    void loadNodes();
    void loadSkins();

    void updateNodes();
    void updateNode(Node* pNode, adt::math::M4 trm);
};
