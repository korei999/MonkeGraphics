#include "Model.hh"

#include "adt/View.hh"
#include "asset.hh"

#include "adt/logs.hh"
#include "frame.hh"

using namespace adt;

Pool<Model, 128> Model::s_poolModels(INIT);

Model::Model(adt::i16 assetModelI)
    : m_modelAssetI(assetModelI)
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);

    /*LOG_BAD("skins.size: {}\n", model.m_vSkins.getSize());*/
    if (model.m_vSkins.empty())
        return;

    m_arena = {SIZE_1K};

    if (model.m_vSkins.empty())
        return;

    const auto& skin = model.m_vSkins[0];
    const auto& joints = skin.vJoints;

    m_vJoints.setSize(&m_arena, joints.getSize());
    m_vJointTrms.setSize(&m_arena, joints.getSize());
    m_mapNodeIToJointI = {&m_arena, model.m_vNodes.getSize()};

    for (auto& joint : m_vJoints) joint = {}; /* default init */
    for (auto& trm : m_vJointTrms) trm = math::M4Iden();

    for (ssize jointI = 0; jointI < joints.getSize(); ++jointI)
    {
        int nodeI = skin.vJoints[jointI];
        m_mapNodeIToJointI.insert(&m_arena, nodeI, jointI);
        LOG_GOOD("insert: nodeI: {}, jointI: {}\n", nodeI, jointI);
        ADT_ASSERT(m_mapNodeIToJointI.search(nodeI).eStatus != MAP_RESULT_STATUS::NOT_FOUND, " ");

        auto& j = m_vJoints[jointI];
        j.parentI = -1;

        const auto& node = model.m_vNodes[nodeI];
        if (node.eTransformationType == gltf::Node::TRANSFORMATION_TYPE::ANIMATION)
        {
            j.translation = node.uTransformation.animation.translation;
            j.rotation = node.uTransformation.animation.rotation;
            j.scale = node.uTransformation.animation.scale;
            LOG_WARN("jointI: [{}], tr: [{}], ro: [{}], sc: [{}]\n", jointI, j.translation, j.rotation, j.scale);
        }
        else if (node.eTransformationType == gltf::Node::TRANSFORMATION_TYPE::MATRIX)
        {
            j.bindTrm = node.uTransformation.matrix;
        }
    }

    // for (ssize parentNodeI = 0; parentNodeI < model.m_vNodes.getSize(); ++parentNodeI)
    // {
    //     const gltf::Node& node = model.m_vNodes[parentNodeI];
    //     for (const auto& childI : node.vChildren)
    //     {
    //         /* TODO: implement unordered_set */
    //         /* if this child is a joint and if nodeI is also a joint, set relationship */
    //         if (joints.search(childI) && joints.search(parentNodeI))
    //         {
    //             int childJointI = m_mapNodeIToJointI.search(childI).valueOr(-1);
    //             int parentJointI = m_mapNodeIToJointI.search(parentNodeI).valueOr(-1);
    //             ADT_ASSERT(childJointI != -1 && parentJointI != -1, " ");

    //             m_vJoints[childJointI].parentI = parentJointI;
    //             m_vJoints[parentJointI].vChildren.push(&m_arena, childJointI);
    //         }
    //     }
    // }

    for (auto& animation : model.m_vAnimations)
    {
        for (auto& sampler : animation.vSamplers)
        {
            auto& accTimeStamps = model.m_vAccessors[sampler.inputI];
            m_globalMinTime = utils::min(m_globalMinTime, static_cast<f64>(accTimeStamps.uMin.SCALAR));
            m_globalMaxTime = utils::max(m_globalMaxTime, static_cast<f64>(accTimeStamps.uMax.SCALAR));
        }
    }

    // if (skin.skeleton >= 0)
    //     m_rootJointI = m_mapNodeIToJointI.search(skin.skeleton).valueOr(-1);
    // else m_rootJointI = 0;

    m_rootJointI = skin.vJoints[0];
    loadJoint(m_rootJointI, -1);

    // if (m_rootJointI == -1)
    // {
    //     for (ssize i = 0; i < m_vJoints.getSize(); ++i)
    //     {
    //         if (m_vJoints[i].parentI == -1)
    //         {
    //             m_rootJointI = static_cast<i16>(i);
    //             break;
    //         }
    //     }
    // }

    LOG_BAD("m_rootJointI: {}\n", m_rootJointI);
}

void
Model::updateGlobalTransforms(adt::i16 jointI, adt::math::M4 parentTrm)
{
    LOG_GOOD("jointI: {}\n", jointI);
    Joint& joint = m_vJoints[jointI];
    joint.globalTrm = parentTrm * 
        (math::M4TranslationFrom(joint.translation) *
         math::QtRot(joint.rotation) *
         math::M4ScaleFrom(joint.scale));

    for (int childI : joint.vChildren)
        updateGlobalTransforms(childI, joint.globalTrm);
}

void
Model::loadJoint(adt::i16 gltfNodeI, adt::i16 parentJointI)
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);

    int currJointI = m_mapNodeIToJointI.search(gltfNodeI).valueOr(-1);
    ADT_ASSERT(currJointI != -1, " ");
    auto& joint = m_vJoints[currJointI];

    joint.parentI = parentJointI;
    ssize nChildren = model.m_vNodes[gltfNodeI].vChildren.getSize();
    joint.vChildren.setSize(&m_arena, nChildren);
    for (ssize childI = 0; childI < nChildren; ++childI)
    {
        int gltfNodeIForChild = model.m_vNodes[gltfNodeI].vChildren[childI];
        joint.vChildren[childI] = m_mapNodeIToJointI.search(gltfNodeIForChild).valueOr(-1);
        ADT_ASSERT(joint.vChildren[childI] != -1, " ");
        loadJoint(gltfNodeIForChild, currJointI);
    }
}

void
Model::updateAnimations()
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);
    /* NOTE: using the first animation */
    const auto& animation = model.m_vAnimations[0];

    m_time = std::fmod(m_time + frame::g_frameTime, m_globalMaxTime);

    int count {};
    for (const auto& channel : animation.vChannels)
    {
        const auto& sampler = animation.vSamplers[channel.samplerI];

        int jointI = m_mapNodeIToJointI.search(channel.target.nodeI).valueOr(-1);
        if (jointI >= 0 && jointI < m_vJoints.getSize())
        {
            Joint& joint = m_vJoints[jointI];
            ++count;

            /*LOG_BAD("nodeI: {}, jointI: {}, path: {}\n", channel.target.nodeI, jointI, channel.target.ePath);*/

            const gltf::Accessor& accTimeStamps = model.m_vAccessors[sampler.inputI];
            const gltf::BufferView& viewTimeStamps = model.m_vBufferViews[accTimeStamps.bufferViewI];
            const gltf::Buffer& buffTimeStamps = model.m_vBuffers[viewTimeStamps.bufferI];

            const View<f32> vwTimeStamps(
                (f32*)(&buffTimeStamps.sBin[accTimeStamps.byteOffset + viewTimeStamps.byteOffset]),
                accTimeStamps.count,
                viewTimeStamps.byteStride
            );

            ADT_ASSERT(vwTimeStamps.getSize() >= 2, " ");

            if (m_time >= accTimeStamps.uMin.SCALAR && m_time <= accTimeStamps.uMax.SCALAR)
            {
                f32 prevTime = -INFINITY;
                f32 nextTime {};

                int prevTimeI = 0;
                for (ssize i = 0; i < vwTimeStamps.getSize(); ++i)
                {
                    const auto& timeStamp = vwTimeStamps[i];
                    if (timeStamp < m_time && timeStamp > prevTime)
                        prevTimeI = i;
                }

                prevTime = vwTimeStamps[prevTimeI + 0];
                nextTime = vwTimeStamps[prevTimeI + 1];

                ADT_ASSERT(nextTime - prevTime != 0.0f, " ");
                const f32 interpolationValue = (m_time - prevTime) / (nextTime - prevTime);

                const auto& accOutput = model.m_vAccessors[sampler.outputI];
                const auto& viewOutput = model.m_vBufferViews[accOutput.bufferViewI];
                const auto& buffOutput = model.m_vBuffers[viewOutput.bufferI];

                if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::TRANSLATION)
                {
                    const View<math::V3> spOutTranslations(
                        reinterpret_cast<const math::V3*>(&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset]),
                        accOutput.count, viewOutput.byteStride
                    );

                    joint.translation = lerp(spOutTranslations[prevTimeI], spOutTranslations[prevTimeI + 1], interpolationValue);
                }
                else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::ROTATION)
                {
                    const View<math::Qt> vwOutRotations(
                        reinterpret_cast<const math::Qt*>(&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset]),
                        accOutput.count, viewOutput.byteStride
                    );

                    math::Qt prevRot = vwOutRotations[prevTimeI + 0];
                    math::Qt nextRot = vwOutRotations[prevTimeI + 1];

                    joint.rotation = slerp(prevRot, nextRot, interpolationValue);
                }
                else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::SCALE)
                {
                    const View<math::V3> spOutScales(
                        reinterpret_cast<const math::V3*>(&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset]),
                        accOutput.count, viewOutput.byteStride
                    );

                    joint.scale = lerp(spOutScales[prevTimeI], spOutScales[prevTimeI + 1], interpolationValue);
                }
            }
        }
    }
    /*LOG_BAD("count: {}\n", count);*/
    /*CERR("\n");*/
}

void
Model::updateSkeletalTransofms(adt::math::M4 trm)
{
    updateGlobalTransforms(m_rootJointI, trm);
    CERR("\n");
    updateJointTransforms();
}

void
Model::update()
{
    if (m_vJoints.empty()) return;

    for (ssize jointI = 0; jointI < m_vJoints.getSize(); ++jointI)
        m_vJointTrms[jointI] = m_vJoints[jointI].getTrm();

    updateJoint(0);

    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);

    const auto& skin = model.m_vSkins[0];
    const auto& accInv = model.m_vAccessors[skin.inverseBindMatricesI];
    const auto& viewInv = model.m_vBufferViews[accInv.bufferViewI];
    const auto& buffInv = model.m_vBuffers[viewInv.bufferI];

    const View<math::M4> vwInv(
        reinterpret_cast<const math::M4*>(&buffInv.sBin[accInv.byteOffset + viewInv.byteOffset]),
        accInv.count, viewInv.byteStride
    );

    ADT_ASSERT(m_vJoints.getSize() == vwInv.getSize() && m_vJointTrms.getSize() == vwInv.getSize(),
        "%lld, %lld, %lld", m_vJoints.getSize(), vwInv.getSize(), m_vJointTrms.getSize()
    );

    for (ssize i = 0; i < m_vJoints.getSize(); ++i)
        m_vJointTrms[i] *= vwInv[i];
}

void
Model::updateJoint(adt::i16 jointI)
{
    LOG_BAD("jointI: {}, root: {}\n", jointI, m_rootJointI);
    auto& joint = m_vJoints[jointI];

    auto parentJointI = joint.parentI;
    if (parentJointI != -1)
        m_vJointTrms[jointI] = m_vJointTrms[parentJointI] * m_vJointTrms[jointI];

    for (ssize childI = 0; childI < joint.vChildren.getSize(); ++childI)
        updateJoint(joint.vChildren[childI]);
}

void
Model::updateJointTransforms()
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);

    const auto& skin = model.m_vSkins[0];
    const auto& accInv = model.m_vAccessors[skin.inverseBindMatricesI];
    const auto& viewInv = model.m_vBufferViews[accInv.bufferViewI];
    const auto& buffInv = model.m_vBuffers[viewInv.bufferI];

    const View<math::M4> vwInv(
        reinterpret_cast<const math::M4*>(&buffInv.sBin[accInv.byteOffset + viewInv.byteOffset]),
        accInv.count, viewInv.byteStride
    );

    ADT_ASSERT(m_vJoints.getSize() == vwInv.getSize() && m_vJointTrms.getSize() == vwInv.getSize(),
        "%lld, %lld, %lld", m_vJoints.getSize(), vwInv.getSize(), m_vJointTrms.getSize()
    );

    for (ssize i = 0; i < m_vJoints.getSize(); ++i)
        m_vJointTrms[i] = m_vJoints[i].globalTrm * vwInv[i];
}
