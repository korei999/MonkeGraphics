#include "Model.hh"

#include "frame.hh"
#include "asset.hh"

#include "adt/logs.hh"
#include "adt/View.hh"

using namespace adt;

Pool<Model, 128> Model::s_poolModels(INIT);

Model::Model(i16 assetModelI)
    : m_modelAssetI(assetModelI)
{
    const gltf::Model& model = gltfModel();

    m_arena = {SIZE_1M};

    for (auto& animation : model.m_vAnimations)
    {
        for (auto& sampler : animation.vSamplers)
        {
            auto& accTimeStamps = model.m_vAccessors[sampler.inputI];
            m_globalMinTime = utils::min(m_globalMinTime, static_cast<f64>(accTimeStamps.uMin.SCALAR));
            m_globalMaxTime = utils::max(m_globalMaxTime, static_cast<f64>(accTimeStamps.uMax.SCALAR));
        }
    }

    loadNodes();
    loadSkins();
}

gltf::Model&
Model::gltfModel() const
{
    return *asset::fromModelI(m_modelAssetI);
}

void
Model::updateAnimation(int animationI)
{
    const gltf::Model& model = gltfModel();

    if (model.m_vAnimations.empty())
    {
        return;
    }
    else if (animationI < 0 || animationI >= model.m_vAnimations.size())
    {
        LOG_WARN("out of range: animationI: {} (size: {})\n", animationI, model.m_vAnimations.size());
        return;
    }

    m_time += frame::g_frameTime;

    const gltf::Animation& animation = model.m_vAnimations[animationI];

    for (const auto& channel : animation.vChannels)
    {
        const auto& sampler = animation.vSamplers[channel.samplerI];
        const gltf::Accessor& accTimeStamps = model.m_vAccessors[sampler.inputI];
        ADT_ASSERT(accTimeStamps.eComponentType == gltf::COMPONENT_TYPE::FLOAT, " ");

        const View<f32> vwTimeStamps = model.accessorView<f32>(sampler.inputI);
        Node& node = m_vNodes[channel.target.nodeI];
        ADT_ASSERT(node.eType == Node::TRANSFORMATION_TYPE::ANIMATION, " ");

        ADT_ASSERT(vwTimeStamps.size() >= 2, " ");

        const f64 actualDuration = accTimeStamps.uMax.SCALAR - accTimeStamps.uMin.SCALAR;
        m_time = std::fmod(m_time - accTimeStamps.uMin.SCALAR, actualDuration) + accTimeStamps.uMin.SCALAR;

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
                const View<math::V3> vwOutTranslations(model.accessorView<math::V3>(sampler.outputI));

                node.uTransform.tra = lerp(vwOutTranslations[prevTimeI], vwOutTranslations[prevTimeI + 1], interpolationValue);
            }
            else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::ROTATION)
            {
                const View<math::Qt> vwOutRotations(model.accessorView<math::Qt>(sampler.outputI));

                math::Qt prevRot = vwOutRotations[prevTimeI + 0];
                math::Qt nextRot = vwOutRotations[prevTimeI + 1];

                node.uTransform.rot = slerp(prevRot, nextRot, interpolationValue);
            }
            else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::SCALE)
            {
                const View<math::V3> vwOutScales(model.accessorView<math::V3>(sampler.outputI));

                node.uTransform.sca = lerp(vwOutScales[prevTimeI], vwOutScales[prevTimeI + 1], interpolationValue);
            }
        }
    }

    updateNodes();
    updateSkins();
}

void
Model::updateSkins()
{
    const gltf::Model& model = gltfModel();

    for (const int& nodeI : m_vSkinnedNodes)
    {
        const gltf::Node& gltfNode = model.m_vNodes[nodeI];
        const gltf::Skin& gltfSkin = model.m_vSkins[gltfNode.skinI];
        const View<math::M4> vwInvBinds = model.accessorView<math::M4>(gltfSkin.inverseBindMatricesI);

        Node& node = m_vNodes[nodeI];
        Skin& skin = m_vSkins[gltfNode.skinI];

        const math::M4 invBindMatrix = math::M4Inv(node.finalTransform);

        for (const int& jointNodeI : gltfSkin.vJoints)
        {
            const int jointI = gltfSkin.vJoints.idx(&jointNodeI);
            const Node& jointNode = m_vNodes[jointNodeI];

            skin.vJointMatrices[jointI] =
                invBindMatrix *
                jointNode.finalTransform *
                vwInvBinds[jointI];
        }
    }
}

void
Model::updateNodes()
{
    const gltf::Model& model = gltfModel();
    const gltf::Scene& scene = model.m_vScenes[model.m_defaultSceneI];

    for (const int gltfNodeI : scene.vNodes)
        updateNode(&m_vNodes[gltfNodeI], math::M4Iden());
}

void
Model::updateNode(Node* pNode, math::M4 trm)
{
    /*math::M4 nodeTrm;*/

    switch (pNode->eType)
    {
        case Node::TRANSFORMATION_TYPE::ANIMATION:
        {
            trm *= math::M4TranslationFrom(pNode->uTransform.tra) *
                math::QtRot(pNode->uTransform.rot) *
                math::M4ScaleFrom(pNode->uTransform.sca);
        }
        break;

        case Node::TRANSFORMATION_TYPE::MATRIX:
        {
            trm *= pNode->uTransform.matrix;
        }
        break;

        default:
        /*nodeTrm = math::M4Iden();*/
        break;
    }

    const gltf::Node& gltfNod = gltfNode(*pNode);

    pNode->finalTransform = trm;

    for (const int childI : gltfNod.vChildren)
        updateNode(&m_vNodes[childI], trm);
}

void
Model::loadNodes()
{
    const gltf::Model& model = gltfModel();

    m_vNodes.setCap(&m_arena, model.m_vNodes.size());

    for (const gltf::Node& gltfNode : model.m_vNodes)
    {
        Node& newNode = m_vNodes[m_vNodes.push(&m_arena, {})];

        if (gltfNode.skinI > -1)
            m_vSkinnedNodes.push(&m_arena, model.m_vNodes.idx(&gltfNode));

        switch (gltfNode.eTransformationType)
        {
            case gltf::Node::TRANSFORMATION_TYPE::ANIMATION:
            {
                newNode.eType = Node::TRANSFORMATION_TYPE::ANIMATION;
                newNode.uTransform.tra = gltfNode.uTransformation.animation.translation;
                newNode.uTransform.rot = gltfNode.uTransformation.animation.rotation;
                newNode.uTransform.sca = gltfNode.uTransformation.animation.scale;
            }
            break;

            case gltf::Node::TRANSFORMATION_TYPE::MATRIX:
            {
                newNode.eType = Node::TRANSFORMATION_TYPE::MATRIX;
                newNode.uTransform.matrix = gltfNode.uTransformation.matrix;
            }
            break;

            case gltf::Node::TRANSFORMATION_TYPE::NONE:
            {
                newNode.eType = Node::TRANSFORMATION_TYPE::NONE;
                newNode.uTransform.matrix = math::M4Iden(); /* just in case */
            }
            break;

            default: ADT_ASSERT(false, "invadil path"); break;
        };
    }
}

void
Model::loadSkins()
{
    const gltf::Model& model = gltfModel();

    m_vSkins.setCap(&m_arena, model.m_vSkins.size());

    for (const auto& gltfSkin : model.m_vSkins)
    {
        Skin& newSkin = m_vSkins[m_vSkins.push(&m_arena, {})];

        newSkin.vJointMatrices.setSize(&m_arena, gltfSkin.vJoints.size());
        for (auto& trm : newSkin.vJointMatrices) trm = {};
    }
}

gltf::Node&
Model::gltfNode(const Node& node) const
{
    return gltfModel().m_vNodes[m_vNodes.idx(&node)];
};
