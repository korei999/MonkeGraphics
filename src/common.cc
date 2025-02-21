#include "common.hh"

#include "asset.hh"
#include "frame.hh"

using namespace adt;

namespace common
{

const Span2D<ImagePixelRGBA> g_spDefaultTexture = createDefaultTexture();

// void
// updateModelNode(animation::Animator* pAnimator, const gltf::Node& node, adt::math::M4* pTrm)
// {
//     using namespace adt::math;
// 
//     const auto& model = asset::g_poolObjects[PoolHandle<asset::Object>(pAnimator->m_gltfModelHandle)].m_uData.model;
//     const ssize nodeI = model.m_vNodes.idx(&node);
// 
//     // pNode->currTime += frame::g_frameTime;
// 
//     if (node.eTransformationType == gltf::Node::TRANSFORMATION_TYPE::MATRIX)
//     {
//         *pTrm *= node.uTransformation.matrix;
//     }
//     else
//     {
//         V3 currTranslation = node.uTransformation.animation.translation;
//         Qt currRotation = node.uTransformation.animation.rotation;
//         V3 currScale = node.uTransformation.animation.scale;
// 
//         f32 globalMinTime {};
//         f32 globalMaxTime {};
// 
//         /* TODO: slow */
//         for (auto& animation : model.m_vAnimations)
//         {
//             for (auto& sampler : animation.vSamplers)
//             {
//                 auto& accTimeStamps = model.m_vAccessors[sampler.inputI];
//                 globalMinTime = utils::min(globalMinTime, static_cast<f32>(accTimeStamps.uMin.SCALAR));
//                 globalMaxTime = utils::max(globalMaxTime, static_cast<f32>(accTimeStamps.uMax.SCALAR));
//             }
//         }
// 
//         for (auto& animation : model.m_vAnimations)
//         {
//             for (auto& channel : animation.vChannels)
//             {
//                 if (channel.target.nodeI != nodeI)
//                     continue;
// 
//                 const gltf::Animation::Sampler& sampler = animation.vSamplers[channel.samplerI];
// 
//                 const gltf::Accessor& accTimeStamps = model.m_vAccessors[sampler.inputI];
//                 const gltf::BufferView& viewTimeStamps = model.m_vBufferViews[accTimeStamps.bufferViewI];
//                 const gltf::Buffer& buffTimeStamps = model.m_vBuffers[viewTimeStamps.bufferI];
// 
//                 const Span<f32> spTimeStamps(
//                     (f32*)(&buffTimeStamps.sBin[accTimeStamps.byteOffset + viewTimeStamps.byteOffset]), accTimeStamps.count
//                 );
// 
//                 ADT_ASSERT(spTimeStamps.getSize() >= 2, " ");
// 
//                 // pNode->currTime = std::fmod(pNode->currTime, globalMaxTime);
// 
//                 // if (pNode->currTime >= accTimeStamps.uMin.SCALAR && pNode->currTime <= accTimeStamps.uMax.SCALAR)
//                 // {
//                 //     f32 prevTime = -INFINITY;
//                 //     f32 nextTime {};
// 
//                 //     int prevTimeI = 0;
//                 //     for (auto& timeStamp : spTimeStamps)
//                 //     {
//                 //         if (timeStamp < pNode->currTime && timeStamp > prevTime)
//                 //             prevTimeI = spTimeStamps.idx(&timeStamp);
//                 //     }
// 
//                 //     prevTime = spTimeStamps[prevTimeI + 0];
//                 //     nextTime = spTimeStamps[prevTimeI + 1];
// 
//                 //     ADT_ASSERT(nextTime - prevTime != 0.0f, " ");
//                 //     const f32 interpolationValue = (pNode->currTime - prevTime) / (nextTime - prevTime);
// 
//                 //     const auto& accOutput = pModel->m_vAccessors[sampler.outputI];
//                 //     const auto& viewOutput = pModel->m_vBufferViews[accOutput.bufferViewI];
//                 //     const auto& buffOutput = pModel->m_vBuffers[viewOutput.bufferI];
// 
//                 //     if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::TRANSLATION)
//                 //     {
//                 //         const Span<V3> spOutTranslations(
//                 //             (V3*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
//                 //             accOutput.count
//                 //         );
// 
//                 //         currTranslation = lerp(spOutTranslations[prevTimeI], spOutTranslations[prevTimeI + 1], interpolationValue);
//                 //     }
//                 //     else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::ROTATION)
//                 //     {
//                 //         const Span<Qt> spOutRotations(
//                 //             (Qt*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
//                 //             accOutput.count
//                 //         );
// 
//                 //         Qt prevRot = spOutRotations[prevTimeI + 0];
//                 //         Qt nextRot = spOutRotations[prevTimeI + 1];
// 
//                 //         currRotation = slerp(prevRot, nextRot, interpolationValue);
//                 //     }
//                 //     else if (channel.target.ePath == gltf::Animation::Channel::Target::PATH_TYPE::SCALE)
//                 //     {
//                 //         const Span<V3> spOutScales(
//                 //             (V3*)&buffOutput.sBin[accOutput.byteOffset + viewOutput.byteOffset],
//                 //             accOutput.count
//                 //         );
// 
//                 //         currScale = lerp(spOutScales[prevTimeI], spOutScales[prevTimeI + 1], interpolationValue);
//                 //     }
//                 // }
//             }
// 
//             *pTrm *= M4TranslationFrom(currTranslation) *
//                 QtRot(currRotation) *
//                 M4ScaleFrom(currScale);
//         }
//     }
// }

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
