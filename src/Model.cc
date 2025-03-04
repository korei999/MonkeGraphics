#include "Model.hh"

#include "frame.hh"
#include "asset.hh"

#include "adt/logs.hh"
#include "adt/View.hh"

using namespace adt;

Pool<Model, 128> Model::s_poolModels(INIT);

Model::Model(adt::i16 assetModelI)
    : m_modelAssetI(assetModelI)
{
    const gltf::Model& model = gltfModel();

    m_arena = {SIZE_1M};

    const gltf::Scene& scene = model.m_vScenes[model.m_defaultSceneI];
    for (const int nodeI : scene.vNodes)
    {
        const gltf::Node& node = model.m_vNodes[nodeI];
        loadNode(nullptr, node, nodeI);
    }

    loadSkins();

    for (auto node : m_vAllNodes)
    {
        if (node->m_skinI > -1)
            node->m_pSkin = m_vSkins[node->m_skinI];

        if (node->m_pMesh)
            node->update();
    }

    for (auto& animation : model.m_vAnimations)
    {
        for (auto& sampler : animation.vSamplers)
        {
            auto& accTimeStamps = model.m_vAccessors[sampler.inputI];
            m_globalMinTime = utils::min(m_globalMinTime, static_cast<f64>(accTimeStamps.uMin.SCALAR));
            m_globalMaxTime = utils::max(m_globalMaxTime, static_cast<f64>(accTimeStamps.uMax.SCALAR));
        }
    }
}

void
Model::updateAnimations(int animationI)
{
    const gltf::Model& model = gltfModel();

    if (model.m_vAnimations.empty()) return;

    if (animationI < 0 || animationI >= model.m_vAnimations.size())
    {
        LOG_WARN("out of range: animationI: {} (size: {})\n", animationI, model.m_vAnimations.size());
        return;
    }

    /* NOTE: using the first animation */
    const auto& animation = model.m_vAnimations[animationI];

    m_time += frame::g_frameTime;

    for (const auto& channel : animation.vChannels)
    {
        const auto& sampler = animation.vSamplers[channel.samplerI];

        Node* joint = nodeFromI(channel.target.nodeI);

        const gltf::Accessor& accTimeStamps = model.m_vAccessors[sampler.inputI];

        const View<f32> vwTimeStamps = model.accessorView<f32>(sampler.inputI);

        ADT_ASSERT(vwTimeStamps.size() >= 2, " ");

        m_time = std::fmod(m_time, accTimeStamps.uMax.SCALAR);

        if (m_time >= accTimeStamps.uMin.SCALAR && m_time <= accTimeStamps.uMax.SCALAR)
        {
            f32 prevTime = -INFINITY;
            f32 nextTime {};

            int prevTimeI = 0;
            for (ssize i = 0; i < vwTimeStamps.size(); ++i)
            {
                const auto& timeStamp = vwTimeStamps[i];
                if (timeStamp < m_time && timeStamp > prevTime)
                    prevTimeI = i;
            }

            prevTime = vwTimeStamps[prevTimeI + 0];
            nextTime = vwTimeStamps[prevTimeI + 1];

            ADT_ASSERT(nextTime - prevTime != 0.0f, " ");
            const f32 interpolationValue = (m_time - prevTime) / (nextTime - prevTime);

            if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::TRANSLATION)
            {
                const View<math::V3> spOutTranslations(model.accessorView<math::V3>(sampler.outputI));

                joint->m_translation = lerp(spOutTranslations[prevTimeI], spOutTranslations[prevTimeI + 1], interpolationValue);
            }
            else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::ROTATION)
            {
                const View<math::Qt> vwOutRotations(model.accessorView<math::Qt>(sampler.outputI));

                math::Qt prevRot = vwOutRotations[prevTimeI + 0];
                math::Qt nextRot = vwOutRotations[prevTimeI + 1];

                joint->m_rotation = slerp(prevRot, nextRot, interpolationValue);
            }
            else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::SCALE)
            {
                const View<math::V3> spOutScales(model.accessorView<math::V3>(sampler.outputI));

                joint->m_scale = lerp(spOutScales[prevTimeI], spOutScales[prevTimeI + 1], interpolationValue);
            }
        }
    }

    for (auto* node : m_vNodes)
        node->update();
}

gltf::Model&
Model::gltfModel() const
{
    return *asset::fromModelI(m_modelAssetI);
}

gltf::Node&
Model::gltfNode(const Node* pNode) const
{
    auto& model = gltfModel();
    return model.m_vNodes[pNode->m_idx];
}

Model::Node*
Model::findNode(const Node* const pParent, int idx) const
{
    if (pParent->m_idx == idx)
        return const_cast<Node*>(pParent);

    for (auto* pChild : pParent->m_vChildren)
    {
        if (Node* pFound = findNode(pChild, idx))
            return pFound;
    }

    return nullptr;
}

Model::Node*
Model::nodeFromI(int idx) const
{
    Node* pFound {};
    for (auto* node : m_vNodes)
    {
        pFound = findNode(node, idx);
        if (pFound) break;
    }

    return pFound;
}

void
Model::loadNode(Node* pParent, const gltf::Node& gltfNode, int nodeI)
{
    const gltf::Model& model = gltfModel();
    Node* pNewNode = m_arena.alloc<Node>(Node{});

    pNewNode->m_idx = nodeI;
    pNewNode->m_pParent = pParent;
    pNewNode->m_skinI = gltfNode.skinI;
    pNewNode->m_svName = gltfNode.sName;

    switch (gltfNode.eTransformationType)
    {
        case gltf::Node::TRANSFORMATION_TYPE::ANIMATION:
        {
            pNewNode->m_translation = gltfNode.uTransformation.animation.translation;
            pNewNode->m_rotation = gltfNode.uTransformation.animation.rotation;
            pNewNode->m_scale = gltfNode.uTransformation.animation.scale;
        }
        break;

        case gltf::Node::TRANSFORMATION_TYPE::MATRIX:
        pNewNode->m_matrix = gltfNode.uTransformation.matrix;
        break;

        default: break;
    }

    for (const int childI : gltfNode.vChildren)
        loadNode(pNewNode, model.m_vNodes[childI], childI);

    if (gltfNode.meshI > -1)
    {
        Mesh* pNewMesh = m_arena.alloc<Mesh>(Mesh{});
        pNewMesh->matrix = pNewNode->m_matrix;

        pNewNode->m_pMesh = pNewMesh;
        pNewNode->m_meshI = gltfNode.meshI;
    }

    if (pParent) pParent->m_vChildren.push(&m_arena, pNewNode);
    else m_vNodes.push(&m_arena, pNewNode);

    m_vAllNodes.push(&m_arena, pNewNode);
}

void
Model::loadSkins()
{
    const gltf::Model& model = gltfModel();

    for (const gltf::Skin& gltfSkin : model.m_vSkins)
    {
        Skin* pNewSkin = m_arena.alloc<Skin>(Skin{});
        pNewSkin->svName = gltfSkin.sName;

        for (const int jointI : gltfSkin.vJoints)
        {
            if (Node* pNode = nodeFromI(jointI))
                pNewSkin->vJointNodes.push(&m_arena, pNode);
        }

        if (gltfSkin.inverseBindMatricesI > -1)
        {
            const View vwInvs = model.accessorView<math::M4>(gltfSkin.inverseBindMatricesI);
            pNewSkin->vInverseBindMatrices.setSize(&m_arena, vwInvs.size());
            for (ssize i = 0; i < vwInvs.size(); ++i)
                pNewSkin->vInverseBindMatrices[i] = vwInvs[i];
        }

        if (pNewSkin->vJointNodes.size() > MAX_JOINTS)
            LOG_BAD("too many joints\n");

        m_vSkins.push(&m_arena, pNewSkin);
    }
}

adt::math::M4
Model::Node::matrix() const
{
    math::M4 m = localMatrix();
    Node* p = m_pParent;
    while (p)
    {
        m = p->localMatrix() * m;
        p = p->m_pParent;
    }

    return m;
};

void
Model::Node::update()
{
    if (m_pMesh)
    {
        math::M4 m = matrix();
        m_pMesh->matrix = m;
        if (m_pSkin)
        {
            math::M4 invTrm = math::M4Inv(m);
            ADT_ASSERT(m_pSkin->vInverseBindMatrices.size() < MAX_JOINTS, " ");
            m_pMesh->vJointMatrices.setSize(m_pSkin->vInverseBindMatrices.size());
            for (ssize i = 0; i < m_pSkin->vInverseBindMatrices.size(); ++i)
            {
                Node* pJointNode = m_pSkin->vJointNodes[i];
                math::M4 jointMat = pJointNode->matrix() * m_pSkin->vInverseBindMatrices[i];
                jointMat = invTrm * jointMat;
                m_pMesh->vJointMatrices[i] = jointMat;
            }
        }

        for (Node* child : m_vChildren)
            child->update();
    }
}
