#pragma once

#include "gltf/Model.hh"

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

    struct Animation
    {
        adt::f64 minTime = INFINITY;
        adt::f64 maxTime = -INFINITY;
    };

    /* */

    adt::Arena m_arena {};

    adt::Vec<Node> m_vNodes {};
    adt::Vec<Skin> m_vSkins {};
    adt::Vec<Animation> m_vAnimations {};
    adt::Vec<int> m_vSkinnedNodes {};

    adt::f64 m_time {};
    adt::i16 m_modelAssetI {};

    int m_animationIUsed = -1;

    /* */

    static adt::Pool<Model, 128> g_poolModels;

    /* */

    static Model& fromI(adt::i16 h) { return g_poolModels[{h}]; }

    /* */

    Model() = default;
    Model(adt::i16 assetModelI);

    /* */

    template<typename ...ARGS>
    static adt::PoolHandle<Model>
    make(ARGS&&... args)
    {
        return g_poolModels.make(std::forward<ARGS>(args)...);
    }

    /* */

    gltf::Model& gltfModel() const;
    gltf::Node& gltfNode(const Node& node) const { return gltfModel().m_vNodes[m_vNodes.idx(&node)]; };

    void updateAnimation(int animationI);
    void updateAnimation() { updateAnimation(m_animationIUsed); }

private:
    void loadNodes();
    void loadSkins();
    void loadAnimations();

    void updateNodes();
    void updateNode(Node* pNode, adt::math::M4 trm);

    void updateSkins();
};
