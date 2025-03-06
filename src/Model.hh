#pragma once

#include "gltf/gltf.hh"

#include "adt/Pool.hh"
#include "adt/Arena.hh"

struct Model
{
    static constexpr int MAX_JOINTS = 128;

    struct Node;

    struct Mesh
    {
        adt::math::M4 matrix = adt::math::M4Iden();
        adt::Array<adt::math::M4, MAX_JOINTS> vJointMatrices {};
    };

    struct Skin
    {
        adt::StringView svName {};
        adt::Vec<adt::math::M4> vInverseBindMatrices {};
        adt::Vec<Node*> vJointNodes {};
        adt::i16 skeletonRootI = -1; /* do we care? */
    };

    struct Skin2
    {
        adt::Vec<adt::math::M4> vJointTransforms {};
    };

    struct Node
    {
        Node* m_pParent {};
        Mesh* m_pMesh {};
        Skin* m_pSkin {};

        adt::Vec<Node*> m_vChildren {};

        adt::math::M4 m_matrix = adt::math::M4Iden();
        adt::math::V3 m_translation {};
        adt::math::Qt m_rotation = adt::math::QtIden();
        adt::math::V3 m_scale {1.0f, 1.0f, 1.0f};

        adt::StringView m_svName {};

        adt::i16 m_meshI = -1;
        adt::i16 m_skinI = -1;

        adt::i16 m_idx = -1;

        /* */

        adt::math::M4 localMatrix() const
        {
            return adt::math::M4TranslationFrom(m_translation) *
                adt::math::QtRot(m_rotation) *
                adt::math::M4ScaleFrom(m_scale) *
                m_matrix;
        }

        adt::math::M4 matrix() const;

        void update();
    };

    /* */

    adt::Arena m_arena {};

    adt::Vec<Node*> m_vNodes {};
    adt::Vec<Node*> m_vAllNodes {};
    adt::Vec<Skin*> m_vSkins {};
    adt::Vec<Mesh> m_vMeshes {};

    adt::f64 m_time {};
    adt::f64 m_globalMinTime {};
    adt::f64 m_globalMaxTime {};
    adt::i16 m_modelAssetI {};

    adt::i16 m_animationI = -1;

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

    void updateAnimation(int animationI);
    void updateAnimation() { updateAnimation(m_animationI); };

    gltf::Model& gltfModel() const;
    gltf::Node& gltfNode(const Node* pNode) const;

private:
    Node* findNode(const Node* pParent, int idx) const;
    Node* nodeFromI(int idx) const;

    void loadNode(Node* pParent, const gltf::Node& gltfNode, int nodeI);
    void loadSkins();
};
