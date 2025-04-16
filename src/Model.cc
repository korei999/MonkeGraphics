#include "Model.hh"

#include "app.hh"
#include "asset.hh"

#include "adt/View.hh"

using namespace adt;

Pool<Model, 128> Model::g_poolModels {};

Model::Model(i16 assetModelI)
    : m_arena(SIZE_1M), m_future {INIT}, m_modelAssetI(assetModelI)
{
    loadNodes();
    loadAnimations();
    loadSkins();
}

gltf::Model&
Model::gltfModel() const
{
    return *asset::fromModelI(m_modelAssetI);
}

void
Model::updateAnimation(int animationI, f64 time)
{
    const gltf::Model& model = gltfModel();

    if (model.m_vAnimations.empty() || (animationI < 0 || animationI >= model.m_vAnimations.size()))
        return;

    m_time = time;

    const gltf::Animation& gltfAnimation = model.m_vAnimations[animationI];
    const Animation& animation = m_vAnimations[animationI];

    for (const auto& channel : gltfAnimation.vChannels)
    {
        const auto& sampler = gltfAnimation.vSamplers[channel.samplerI];
        const gltf::Accessor& accTimeStamps = model.m_vAccessors[sampler.inputI];
        ADT_ASSERT(accTimeStamps.eComponentType == gltf::COMPONENT_TYPE::FLOAT, " ");

        const View<f32> vwTimeStamps = model.accessorView<f32>(sampler.inputI);
        ADT_ASSERT(vwTimeStamps.size() >= 2, " ");

        Node& node = m_vNodes[channel.target.nodeI];
        ADT_ASSERT(node.eType == Node::TRANSFORMATION_TYPE::ANIMATION, " ");

        /* NOTE: not sure we need this branch */
        if (m_vSkinnedNodes.empty())
        {
            const f64 actualDuration = animation.maxTime - animation.minTime;
            m_time = std::fmod(m_time - animation.minTime, actualDuration) + animation.minTime;
        }
        else
        {
            const f64 actualDuration = accTimeStamps.uMax.SCALAR - accTimeStamps.uMin.SCALAR;
            m_time = std::fmod(m_time - accTimeStamps.uMin.SCALAR, actualDuration) + accTimeStamps.uMin.SCALAR;
        }

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

            if (prevTimeI + 1 < vwTimeStamps.size()) [[likely]]
            {
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

        const Node& node = m_vNodes[nodeI];
        Skin& skin = m_vSkins[gltfNode.skinI];

        const math::M4 invBindMatrix = math::M4Inv(node.finalTransform);
        /*const math::M4 invBindMatrix = math::M4Iden();*/

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
        Node& newNode = m_vNodes[m_vNodes.emplace(&m_arena)];

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

            default: ADT_ASSERT(false, "invalid path"); break;
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
        Skin& newSkin = m_vSkins[m_vSkins.emplace(&m_arena)];

        newSkin.vJointMatrices.setSize(&m_arena, gltfSkin.vJoints.size());
        for (auto& trm : newSkin.vJointMatrices) trm = {};
    }
}

void
Model::loadAnimations()
{
    const gltf::Model& model = gltfModel();

    m_vAnimations.setCap(&m_arena, model.m_vAnimations.size());

    for (const gltf::Animation& gltfAnim : model.m_vAnimations)
    {
        Animation& newAnim = m_vAnimations[m_vAnimations.emplace(&m_arena)];

        for (const gltf::Animation::Sampler& sampler : gltfAnim.vSamplers)
        {
            auto& accTimeStamps = model.m_vAccessors[sampler.inputI];

            if (newAnim.maxTime < accTimeStamps.uMax.SCALAR)
                newAnim.maxTime = accTimeStamps.uMax.SCALAR;

            if (newAnim.minTime > accTimeStamps.uMin.SCALAR)
                newAnim.minTime = accTimeStamps.uMin.SCALAR;
        }

        if (gltfAnim.sName.empty())
        {
            constexpr StringView svPrefix = "animation";
            Span<char> sp = app::gtl_scratch.nextMemZero<char>(svPrefix.size() + 5);
            const ssize n = print::toSpan(sp, "{}{}", "animation", model.m_vAnimations.idx(&gltfAnim));
            newAnim.sName = String(&m_arena, sp.data(), n);
        }
        else
        {
            newAnim.sName = String(&m_arena, gltfAnim.sName);
        }
    }
}
