#include "game.hh"
#include "Entity.hh"

#include "asset.hh"
#include "control.hh"

#include "adt/logs.hh"

using namespace adt;

namespace game
{

EntityPoolSOA<128> g_aEntites(adt::INIT);

static const String s_asAssetsToLoad[] {
    "assets/duck/Duck.gltf",
    "assets/BoxAnimated/BoxAnimated.gltf",
};

void
loadStuff()
{
    for (const auto& sPath : s_asAssetsToLoad)
    {
        if (!asset::load(sPath))
            LOG_BAD("failed to load: '{}'\n", sPath);
    }

    auto hTest = g_aEntites.makeDefault();
    auto bind = g_aEntites[hTest];

    auto* pObj = asset::search("assets/BoxAnimated/BoxAnimated.gltf", asset::Object::TYPE::MODEL);
    auto idx = asset::g_aObjects.idx(pObj);
    bind.assetI = idx;
}

void
updateState(adt::Arena*)
{
    using namespace control;

    g_camera.updatePos();

    auto what = g_aEntites[{0}];
    what.pos = {std::sinf(frame::g_time), 0.0f, std::cosf(frame::g_time)};
}

void
updateModelNode(gltf::Model* pModel, gltf::Node* pNode, adt::math::M4* pTrm)
{
    using namespace adt::math;

    const ssize nodeI = pModel->m_vNodes.idx(pNode);

    pNode->extras.currTime += frame::g_frameTime;

    if (pNode->eTransformationType == gltf::Node::TRANSFORMATION_TYPE::MATRIX)
    {
        *pTrm *= pNode->uTransformation.matrix;
    }
    else
    {
        V3 currTranslation = pNode->uTransformation.animation.translation;
        Qt currRotation = pNode->uTransformation.animation.rotation;
        V3 currScale = pNode->uTransformation.animation.scale;

        f32 globalMinTime {};
        f32 globalMaxTime {};

        for (auto& animation : pModel->m_vAnimations)
        {
            for (auto& sampler : animation.vSamplers)
            {
                auto& accTimeStamps = pModel->m_vAccessors[sampler.inputI];
                globalMinTime = utils::min(globalMinTime, static_cast<f32>(accTimeStamps.min.SCALAR));
                globalMaxTime = utils::max(globalMaxTime, static_cast<f32>(accTimeStamps.max.SCALAR));
            }
        }

        for (auto& animation : pModel->m_vAnimations)
        {
            for (auto& channel : animation.vChannels)
            {
                if (channel.target.nodeI != nodeI)
                    continue;

                const gltf::Animation::Sampler& sampler = animation.vSamplers[channel.samplerI];

                auto& accTimeStamps = pModel->m_vAccessors[sampler.inputI];
                auto& viewTimeStamps = pModel->m_vBufferViews[accTimeStamps.bufferViewI];
                auto& buffTimeStamps = pModel->m_vBuffers[viewTimeStamps.bufferI];

                const Span<f32> spTimeStamps(
                    (f32*)(&buffTimeStamps.sBin[accTimeStamps.byteOffset + viewTimeStamps.byteOffset]), accTimeStamps.count
                );

                ADT_ASSERT(spTimeStamps.getSize() >= 2, " ");

                pNode->extras.currTime = std::fmod(pNode->extras.currTime, globalMaxTime);

                if (pNode->extras.currTime >= accTimeStamps.min.SCALAR && pNode->extras.currTime <= accTimeStamps.max.SCALAR)
                {
                    f32 prevTime = -INFINITY;
                    f32 nextTime {};

                    int prevTimeI = 0;
                    for (auto& timeStamp : spTimeStamps)
                    {
                        if (timeStamp < pNode->extras.currTime && timeStamp > prevTime)
                            prevTimeI = spTimeStamps.idx(&timeStamp);
                    }

                    prevTime = spTimeStamps[prevTimeI + 0];
                    nextTime = spTimeStamps[prevTimeI + 1];

                    ADT_ASSERT(nextTime - prevTime != 0.0f, " ");
                    const f32 interpolationValue = (pNode->extras.currTime - prevTime) / (nextTime - prevTime);

                    const auto& accOutput = pModel->m_vAccessors[sampler.outputI];
                    const auto& viewOutput = pModel->m_vBufferViews[accOutput.bufferViewI];
                    const auto& buffOutput = pModel->m_vBuffers[viewOutput.bufferI];

                    if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::TRANSLATION)
                    {
                        const Span<V3> spOutTranslations(
                            (V3*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
                            accOutput.count
                        );

                        currTranslation = lerp(spOutTranslations[prevTimeI], spOutTranslations[prevTimeI + 1], interpolationValue);
                    }
                    else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::ROTATION)
                    {
                        const Span<Qt> spOutRotations(
                            (Qt*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
                            accOutput.count
                        );

                        Qt prevRot = spOutRotations[prevTimeI + 0];
                        Qt nextRot = spOutRotations[prevTimeI + 1];

                        currRotation = slerp(prevRot, nextRot, interpolationValue);
                    }
                    else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::SCALE)
                    {
                        const Span<V3> spOutScales(
                            (V3*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
                            accOutput.count
                        );

                        currScale = lerp(spOutScales[prevTimeI], spOutScales[prevTimeI + 1], interpolationValue);
                    }
                }
            }

            *pTrm *= M4TranslationFrom(currTranslation) *
                QtRot(currRotation) *
                M4ScaleFrom(currScale);
        }
    }
}

} /* namespace game */
