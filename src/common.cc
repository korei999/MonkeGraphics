#include "common.hh"

#include "Model.hh"
#include "adt/View.hh"
#include "asset.hh"

using namespace adt;

namespace common
{

const Span2D<ImagePixelRGBA> g_spDefaultTexture = createDefaultTexture();

void
updateModelNode(Model* pModel, const gltf::Node& node, adt::math::M4* pTrm)
{
    using namespace adt::math;

    const auto& model = asset::g_poolObjects[PoolHandle<asset::Object>(pModel->m_modelAssetI)].m_uData.model;
    const ssize nodeI = model.m_vNodes.idx(&node);

    if (node.eTransformationType == gltf::Node::TRANSFORMATION_TYPE::MATRIX)
    {
        *pTrm *= node.uTransformation.matrix;
    }
    else
    {
        V3 currTranslation = node.uTransformation.animation.translation;
        Qt currRotation = node.uTransformation.animation.rotation;
        V3 currScale = node.uTransformation.animation.scale;

        for (auto& animation : model.m_vAnimations)
        {
            for (auto& channel : animation.vChannels)
            {
                if (channel.target.nodeI != nodeI)
                    continue;

                const gltf::Animation::Sampler& sampler = animation.vSamplers[channel.samplerI];

                const gltf::Accessor& accTimeStamps = model.m_vAccessors[sampler.inputI];
                const gltf::BufferView& viewTimeStamps = model.m_vBufferViews[accTimeStamps.bufferViewI];
                const gltf::Buffer& buffTimeStamps = model.m_vBuffers[viewTimeStamps.bufferI];

                const View<f32> spTimeStamps(
                    (f32*)(&buffTimeStamps.sBin[accTimeStamps.byteOffset + viewTimeStamps.byteOffset]),
                    accTimeStamps.count, viewTimeStamps.byteStride
                );

                ADT_ASSERT(spTimeStamps.getSize() >= 2, " ");

                if (pModel->m_time >= accTimeStamps.uMin.SCALAR && pModel->m_time <= accTimeStamps.uMax.SCALAR)
                {
                    f32 prevTime = -INFINITY;
                    f32 nextTime {};

                    int prevTimeI = 0;
                    for (auto& timeStamp : spTimeStamps)
                    {
                        if (timeStamp < pModel->m_time && timeStamp > prevTime)
                            prevTimeI = spTimeStamps.idx(&timeStamp);
                    }

                    prevTime = spTimeStamps[prevTimeI + 0];
                    nextTime = spTimeStamps[prevTimeI + 1];

                    ADT_ASSERT(nextTime - prevTime != 0.0f, " ");
                    const f32 interpolationValue = (pModel->m_time - prevTime) / (nextTime - prevTime);

                    const auto& accOutput = model.m_vAccessors[sampler.outputI];
                    const auto& viewOutput = model.m_vBufferViews[accOutput.bufferViewI];
                    const auto& buffOutput = model.m_vBuffers[viewOutput.bufferI];

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

Span2D<ImagePixelRGBA>
createDefaultTexture()
{
    const int width = 8;
    const int height = 8;

    static ImagePixelRGBA aPixels[width * height] {};

    Span2D sp(aPixels, width, height, width);
    for (int y = 0; y < sp.getHeight(); ++y)
    {
        for (int x = 0; x < sp.getWidth(); ++x)
        {
            u32 colorChannel = 255 * ((x + (y % 2)) % 2);
            ImagePixelRGBA p;
            p.r = 128;
            p.g = static_cast<u8>(colorChannel);
            p.b = 0;
            p.a = 255;

            sp(x, y) = p;
        }
    }

    return sp;
}

} /* namespace common */
