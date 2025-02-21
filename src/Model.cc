#include "Model.hh"

#include "asset.hh"

#include "adt/logs.hh"
#include "frame.hh"

using namespace adt;

Pool<Model, 128> Model::s_poolModels(INIT);

Model::Model(adt::i16 modelAssetI)
    : m_modelAssetI(modelAssetI)
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);

    LOG_BAD("skins.size: {}\n", model.m_vSkins.getSize());
    if (model.m_vSkins.empty())
        return;

    m_arena = {SIZE_1K};

    if (model.m_vSkins.empty())
        return;

    const auto& skin = model.m_vSkins[0];
    const auto& joints = skin.vJoints;

    m_vJoints.setSize(&m_arena, joints.getSize());
    m_vJointsTrms.setSize(&m_arena, joints.getSize());

    for (auto& joint : m_vJoints) joint = {}; /* default init */

    for (ssize jointI = 0; jointI < joints.getSize(); ++jointI)
    {
        int nodeI = skin.vJoints[jointI];
        m_mapNodeIToJointI.insert(&m_arena, nodeI, jointI);
        LOG_BAD("nodeI: {}, jointI: {}\n", nodeI, jointI);

        auto& j = m_vJoints[jointI];
        j.parentI = -1;
        /* TODO: load local transforms */
    }

    for (ssize parentNodeI = 0; parentNodeI < model.m_vNodes.getSize(); ++parentNodeI)
    {
        const gltf::Node& node = model.m_vNodes[parentNodeI];
        for (const auto& childI : node.vChildren)
        {
            /* TODO: implement unordered_set */
            /* if this child is a joint */
            if (joints.contains(childI))
            {
                /* and if nodeI is also a joint, set relationship */
                if (joints.contains(parentNodeI))
                {
                    int childJointI = m_mapNodeIToJointI.search(childI).valueOr(-1);
                    int parentJointI = m_mapNodeIToJointI.search(parentNodeI).valueOr(-1);
                    LOG_BAD("node: {}, childJointI: {}, parentJointI: {}\n", parentNodeI, childJointI, parentJointI);
                    ADT_ASSERT(childJointI != -1 && parentJointI != -1, " ");

                    m_vJoints[childJointI].parentI = parentJointI;
                    m_vJoints[parentJointI].vChildren.push(&m_arena, childJointI);
                }
            }
        }
    }
}

void
Model::updateGlobalTransforms(adt::i16 jointI, const adt::math::M4& parentTrm)
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);

    Joint& joint = m_vJoints[jointI];
    joint.globalTrm = parentTrm * 
        (math::M4TranslationFrom(joint.translation) *
         math::QtRot(joint.rotation) *
         math::M4ScaleFrom(joint.scale));

    /*joint.globalTrm = parentTrm * */
    /*    (math::M4ScaleFrom(joint.scale) **/
    /*     math::QtRot(joint.rotation) **/
    /*     math::M4TranslationFrom(joint.translation));*/

    for (int childI : joint.vChildren)
        updateGlobalTransforms(childI, joint.globalTrm);
}

void
Model::updateAnimations()
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);
    /* NOTE: using the first animation */
    const auto& animation = model.m_vAnimations[0];

    f64 globalMinTime {};
    f64 globalMaxTime {};

    for (auto& sampler : animation.vSamplers)
    {
        auto& accTimeStamps = model.m_vAccessors[sampler.inputI];
        globalMinTime = utils::min(globalMinTime, static_cast<f64>(accTimeStamps.uMin.SCALAR));
        globalMaxTime = utils::max(globalMaxTime, static_cast<f64>(accTimeStamps.uMax.SCALAR));
    }

    for (const auto& channel : animation.vChannels)
    {
        const auto& sampler = animation.vSamplers[channel.samplerI];

        int jointI = m_mapNodeIToJointI.search(channel.target.nodeI).valueOr(-1);
        if (jointI >= 0 && jointI < m_vJoints.getSize())
        {
            Joint& joint = m_vJoints[jointI];

            const gltf::Accessor& accTimeStamps = model.m_vAccessors[sampler.inputI];
            const gltf::BufferView& viewTimeStamps = model.m_vBufferViews[accTimeStamps.bufferViewI];
            const gltf::Buffer& buffTimeStamps = model.m_vBuffers[viewTimeStamps.bufferI];

            const Span<f32> spTimeStamps(
                (f32*)(&buffTimeStamps.sBin[accTimeStamps.byteOffset + viewTimeStamps.byteOffset]), accTimeStamps.count
            );

            ADT_ASSERT(spTimeStamps.getSize() >= 2, " ");

            joint.time = std::fmod(joint.time + frame::g_frameTime, globalMaxTime);

            if (joint.time >= accTimeStamps.uMin.SCALAR && joint.time <= accTimeStamps.uMax.SCALAR)
            {
                f32 prevTime = -INFINITY;
                f32 nextTime {};

                int prevTimeI = 0;
                for (auto& timeStamp : spTimeStamps)
                {
                    if (timeStamp < joint.time && timeStamp > prevTime)
                        prevTimeI = spTimeStamps.idx(&timeStamp);
                }

                prevTime = spTimeStamps[prevTimeI + 0];
                nextTime = spTimeStamps[prevTimeI + 1];

                ADT_ASSERT(nextTime - prevTime != 0.0f, " ");
                const f32 interpolationValue = (joint.time - prevTime) / (nextTime - prevTime);

                const auto& accOutput = model.m_vAccessors[sampler.outputI];
                const auto& viewOutput = model.m_vBufferViews[accOutput.bufferViewI];
                const auto& buffOutput = model.m_vBuffers[viewOutput.bufferI];

                if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::TRANSLATION)
                {
                    const Span<math::V3> spOutTranslations(
                        (math::V3*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
                        accOutput.count
                    );

                    joint.translation = lerp(spOutTranslations[prevTimeI], spOutTranslations[prevTimeI + 1], interpolationValue);
                }
                else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::ROTATION)
                {
                    const Span<math::Qt> spOutRotations(
                        (math::Qt*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
                        accOutput.count
                    );

                    math::Qt prevRot = spOutRotations[prevTimeI + 0];
                    math::Qt nextRot = spOutRotations[prevTimeI + 1];

                    joint.rotation = slerp(prevRot, nextRot, interpolationValue);
                }
                else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::SCALE)
                {
                    const Span<math::V3> spOutScales(
                        (math::V3*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
                        accOutput.count
                    );

                    joint.scale = lerp(spOutScales[prevTimeI], spOutScales[prevTimeI + 1], interpolationValue);
                }
            }
        }
    }
}

void
Model::updateJointTransforms()
{
    const gltf::Model& model = *asset::fromModelI(m_modelAssetI);

    const auto& skin = model.m_vSkins[0];
    const auto& accInv = model.m_vAccessors[skin.inverseBindMatricesI];
    const auto& viewInv = model.m_vBufferViews[accInv.bufferViewI];
    const auto& buffInv = model.m_vBuffers[viewInv.bufferI];

    const Span<math::M4> spInv(
        (math::M4*)(&buffInv.sBin[accInv.byteOffset + viewInv.byteOffset]),
        accInv.count
    );
    LOG_GOOD("spInv: {}\n", spInv);

    ADT_ASSERT(m_vJoints.getSize() == spInv.getSize() && m_vJointsTrms.getSize() == spInv.getSize(),
        "%lld, %lld, %lld", m_vJoints.getSize(), spInv.getSize(), m_vJointsTrms.getSize()
    );

    for (ssize i = 0; i < m_vJoints.getSize(); ++i)
        m_vJointsTrms[i] = m_vJoints[i].globalTrm * spInv[i];
}
