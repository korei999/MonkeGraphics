#include "sw.hh"

#include "app.hh"
#include "asset.hh"
#include "clip.hh"
#include "common.hh"
#include "control.hh"
#include "game/game.hh"
#include "gltf/gltf.hh"

#include "adt/file.hh"

using namespace adt;

namespace render::sw
{

enum class SAMPLER : u8 { NEAREST, BILINEAR };

struct IndexU16x3 { u16 x, y, z; };
struct IndexU32x3 { u32 x, y, z; };

static math::V2
ndcToPix(math::V2 ndcPos)
{
    using namespace adt::math;
    const auto& win = *app::g_pWindow;

    V2 res = 0.5f * (ndcPos + V2{1.0f, 1.0f});
    res *= V2{
        static_cast<f32>(win.m_width),
        static_cast<f32>(win.m_height)
    };

    return res;
}

[[maybe_unused]] ADT_NO_UB static void
drawTriangleSSE(
    clip::Vertex vertex0, clip::Vertex vertex1, clip::Vertex vertex2,
    const Span2D<ImagePixelRGBA> spTexture,
    const SAMPLER eSampler
)
{
    using namespace adt::math;

    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();
    Span2D spDepth = win.depthBuffer();

    vertex0.pos.w = 1.0f / vertex0.pos.w;
    vertex1.pos.w = 1.0f / vertex1.pos.w;
    vertex2.pos.w = 1.0f / vertex2.pos.w;

    vertex0.pos.xyz *= vertex0.pos.w;
    vertex1.pos.xyz *= vertex1.pos.w;
    vertex2.pos.xyz *= vertex2.pos.w;

    vertex0.uv *= vertex0.pos.w;
    vertex1.uv *= vertex1.pos.w;
    vertex2.uv *= vertex2.pos.w;

    const V2 fPointA = ndcToPix(vertex0.pos.xy);
    const V2 fPointB = ndcToPix(vertex1.pos.xy);
    const V2 fPointC = ndcToPix(vertex2.pos.xy);

    int minX = utils::min(
        utils::min(static_cast<int>(fPointA.x), static_cast<int>(fPointB.x)),
        static_cast<int>(fPointC.x)
    );
    int maxX = utils::max(
        utils::max(static_cast<int>(std::round(fPointA.x)), static_cast<int>(std::round(fPointB.x))),
        static_cast<int>(std::round(fPointC.x))
    );
    int minY = utils::min(
        utils::min(static_cast<int>(fPointA.y), static_cast<int>(fPointB.y)),
        static_cast<int>(fPointC.y)
    );
    int maxY = utils::max(
        utils::max(static_cast<int>(std::round(fPointA.y)), static_cast<int>(std::round(fPointB.y))),
        static_cast<int>(std::round(fPointC.y))
    );

    minX = utils::clamp(minX, 0, static_cast<int>(sp.getWidth() - 1));
    maxX = utils::clamp(maxX, 0, static_cast<int>(sp.getWidth() - 1));
    minY = utils::clamp(minY, 0, static_cast<int>(sp.getHeight() - 1));
    maxY = utils::clamp(maxY, 0, static_cast<int>(sp.getHeight() - 1));

    const IV2 pointA = IV2_F24_8(fPointA);
    const IV2 pointB = IV2_F24_8(fPointB);
    const IV2 pointC = IV2_F24_8(fPointC);

    const IV2 edge0 = pointB - pointA;
    const IV2 edge1 = pointC - pointB;
    const IV2 edge2 = pointA - pointC;

    /* discard backfaces early */
    if (IV2Cross(edge0, pointC - pointA) > 0)
        return;

    const bool bTopLeft0 = (edge0.y > 0) || (edge0.x > 0 && edge0.y == 0);
    const bool bTopLeft1 = (edge1.y > 0) || (edge1.x > 0 && edge1.y == 0);
    const bool bTopLeft2 = (edge2.y > 0) || (edge2.x > 0 && edge2.y == 0);

    const simd::f32x4 barycentricDiv = 256.0f / static_cast<f32>(IV2Cross(pointB - pointA, pointC - pointA));

    simd::i32x4 edge0DiffX = edge0.y;
    simd::i32x4 edge1DiffX = edge1.y;
    simd::i32x4 edge2DiffX = edge2.y;

    const simd::i32x4 edge0DiffY = -edge0.x;
    const simd::i32x4 edge1DiffY = -edge1.x;
    const simd::i32x4 edge2DiffY = -edge2.x;

    simd::i32x4 edge0RowY {};
    simd::i32x4 edge1RowY {};
    simd::i32x4 edge2RowY {};
    {
        IV2 startPos = IV2_F24_8(V2From(minX, minY) + V2{0.5f, 0.5f});
        i64 edge0RowY64 = IV2Cross(startPos - pointA, edge0);
        i64 edge1RowY64 = IV2Cross(startPos - pointB, edge1);
        i64 edge2RowY64 = IV2Cross(startPos - pointC, edge2);

        i32 edge0RowY32 = i32((edge0RowY64 + math::sign(edge0RowY64)*128) / 256) - (bTopLeft0 ? 0 : -1);
        i32 edge1RowY32 = i32((edge1RowY64 + math::sign(edge1RowY64)*128) / 256) - (bTopLeft1 ? 0 : -1);
        i32 edge2RowY32 = i32((edge2RowY64 + math::sign(edge2RowY64)*128) / 256) - (bTopLeft2 ? 0 : -1);

        edge0RowY = simd::i32x4(edge0RowY32) + simd::i32x4(0, 1, 2, 3) * edge0DiffX;
        edge1RowY = simd::i32x4(edge1RowY32) + simd::i32x4(0, 1, 2, 3) * edge1DiffX;
        edge2RowY = simd::i32x4(edge2RowY32) + simd::i32x4(0, 1, 2, 3) * edge2DiffX;
    }

    edge0DiffX *= 4;
    edge1DiffX *= 4;
    edge2DiffX *= 4;

    for (int y = minY; y <= maxY; ++y)
    {
        simd::i32x4 edge0RowX = edge0RowY;
        simd::i32x4 edge1RowX = edge1RowY;
        simd::i32x4 edge2RowX = edge2RowY;

        for (int x = minX; x <= maxX; x += 4)
        {
            i32* pColor = &sp(x, y).iData;
            f32* pDepth = &spDepth(x, y);
            const simd::i32x4 pixelColors = simd::i32x4Load(pColor);
            const simd::f32x4 pixelDepths = simd::f32x4Load(pDepth);

            simd::i32x4 edgeMask = (edge0RowX | edge1RowX | edge2RowX) >= 0;

            if (simd::moveMask8(edgeMask) != 0)
            {
                const simd::f32x4 t0 = -simd::f32x4(edge1RowX) * barycentricDiv;
                const simd::f32x4 t1 = -simd::f32x4(edge2RowX) * barycentricDiv;
                const simd::f32x4 t2 = -simd::f32x4(edge0RowX) * barycentricDiv;

                const simd::f32x4 depthZ = vertex0.pos.z + t1*(vertex1.pos.z - vertex0.pos.z) + t2*(vertex2.pos.z - vertex0.pos.z);
                const simd::i32x4 depthMask = simd::i32x4Reinterpret(depthZ < pixelDepths);

                const simd::f32x4 oneOverW = t0*vertex0.pos.w + t1*vertex1.pos.w + t2*vertex2.pos.w;

                simd::V2x4 uv = t0*vertex0.uv + t1*vertex1.uv + t2*vertex2.uv;
                uv /= oneOverW;

                simd::i32x4 texelColor {};

                switch (eSampler)
                {
                    case SAMPLER::NEAREST:
                    {
                        simd::i32x4 texelX = simd::i32x4(simd::floor(uv.x * (spTexture.getWidth() - 1)));
                        simd::i32x4 texelY = simd::i32x4(simd::floor(uv.y * (spTexture.getHeight() - 1)));

                        const simd::i32x4 texelMask = (
                            (texelX >= 0) & (texelX < spTexture.getWidth()) &
                            (texelY >= 0) & (texelY < spTexture.getHeight())
                        );

                        texelX = simd::max(simd::min(texelX, spTexture.getWidth() - 1), 0);
                        texelY = simd::max(simd::min(texelY, spTexture.getHeight() - 1), 0);
                        simd::i32x4 texelOffsets = texelY * spTexture.getWidth() + texelX;

                        const simd::i32x4 trueCase = simd::i32x4Gather((i32*)spTexture.data(), texelOffsets);
                        const simd::i32x4 falseCase = 0xff00ff00;

                        texelColor = (trueCase & texelMask) + simd::andNot(texelMask, falseCase);
                    }
                    break;

                    case SAMPLER::BILINEAR:
                    {
                        simd::V2x4 texelV2 = uv *
                            V2From(spTexture.getWidth(), spTexture.getHeight()) -
                            V2{0.5f, 0.5f};

                        simd::IV2x4 aTexelPos[4] {};
                        aTexelPos[0] = simd::IV2x4(simd::floor(texelV2.x), simd::floor(texelV2.y));
                        aTexelPos[1] = aTexelPos[0] + IV2{1, 0};
                        aTexelPos[2] = aTexelPos[0] + IV2{0, 1};
                        aTexelPos[3] = aTexelPos[0] + IV2{1, 1};

                        simd::V3x4 aTexelColors[4] {};
                        for (int texelI = 0; texelI < utils::size(aTexelPos); ++texelI)
                        {
                            simd::IV2x4 currTexelPos = aTexelPos[texelI];
                            {
                                simd::V2x4 currTexelPosF = simd::V2x4(currTexelPos);
                                simd::V2x4 factor = simd::floor(currTexelPosF / V2From(spTexture.getWidth(), spTexture.getHeight()));
                                currTexelPosF = currTexelPosF - factor * V2From(spTexture.getWidth(), spTexture.getHeight());
                                currTexelPos = simd::IV2x4(currTexelPosF);
                            }

                            simd::i32x4 texelOffsets = currTexelPos.y * spTexture.getWidth() + currTexelPos.x;
                            simd::i32x4 loadMask = edgeMask & depthMask;
                            texelOffsets = (texelOffsets & loadMask) + simd::andNot(loadMask, simd::i32x4(0));
                            simd::i32x4 texelColorI32 = simd::i32x4Gather((i32*)spTexture.data(), texelOffsets);

                            aTexelColors[texelI] = colorI32x4ToV3x4(texelColorI32);
                        }

                        simd::f32x4 s = texelV2.x - simd::floor(texelV2.x);
                        simd::f32x4 k = texelV2.y - simd::floor(texelV2.y);

                        simd::V3x4 interpolated0 = lerp(aTexelColors[0], aTexelColors[1], s);
                        simd::V3x4 interpolated1 = lerp(aTexelColors[2], aTexelColors[3], s);
                        simd::V3x4 finalColor = lerp(interpolated0, interpolated1, k);

                        texelColor = colorV3x4ToI32x4(finalColor);
                    }
                    break;
                }

                const simd::i32x4 finalMaskI32 = edgeMask & depthMask;
                const simd::f32x4 finalMaskF32 = simd::f32x4Reinterpret(finalMaskI32);
                const simd::i32x4 outputColors = (texelColor & finalMaskI32) + simd::andNot(finalMaskI32, pixelColors);
                const simd::f32x4 outputDepth = (depthZ & finalMaskF32) + simd::andNot(finalMaskF32, pixelDepths);

                simd::i32x4Store(pColor, outputColors);
                simd::f32x4Store(pDepth, outputDepth);
            }
            edge0RowX += edge0DiffX;
            edge1RowX += edge1DiffX;
            edge2RowX += edge2DiffX;
        }
        edge0RowY += edge0DiffY;
        edge1RowY += edge1DiffY;
        edge2RowY += edge2DiffY;
    }
}

#ifdef ADT_AVX2

ADT_NO_UB static void
drawTriangleAVX2(
    clip::Vertex vertex0, clip::Vertex vertex1, clip::Vertex vertex2,
    const Span2D<ImagePixelRGBA> spTexture,
    const SAMPLER eSampler
)
{
    using namespace adt::math;

    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();
    Span2D spDepth = win.depthBuffer();

    vertex0.pos.w = 1.0f / vertex0.pos.w;
    vertex1.pos.w = 1.0f / vertex1.pos.w;
    vertex2.pos.w = 1.0f / vertex2.pos.w;

    vertex0.pos.xyz *= vertex0.pos.w;
    vertex1.pos.xyz *= vertex1.pos.w;
    vertex2.pos.xyz *= vertex2.pos.w;

    vertex0.uv *= vertex0.pos.w;
    vertex1.uv *= vertex1.pos.w;
    vertex2.uv *= vertex2.pos.w;

    const V2 fPointA = ndcToPix(vertex0.pos.xy);
    const V2 fPointB = ndcToPix(vertex1.pos.xy);
    const V2 fPointC = ndcToPix(vertex2.pos.xy);

    int minX = utils::min(
        utils::min(static_cast<int>(fPointA.x), static_cast<int>(fPointB.x)),
        static_cast<int>(fPointC.x)
    );
    int maxX = utils::max(
        utils::max(static_cast<int>(std::round(fPointA.x)), static_cast<int>(std::round(fPointB.x))),
        static_cast<int>(std::round(fPointC.x))
    );
    int minY = utils::min(
        utils::min(static_cast<int>(fPointA.y), static_cast<int>(fPointB.y)),
        static_cast<int>(fPointC.y)
    );
    int maxY = utils::max(
        utils::max(static_cast<int>(std::round(fPointA.y)), static_cast<int>(std::round(fPointB.y))),
        static_cast<int>(std::round(fPointC.y))
    );

    minX = utils::clamp(minX, 0, static_cast<int>(sp.getWidth() - 1));
    maxX = utils::clamp(maxX, 0, static_cast<int>(sp.getWidth() - 1));
    minY = utils::clamp(minY, 0, static_cast<int>(sp.getHeight() - 1));
    maxY = utils::clamp(maxY, 0, static_cast<int>(sp.getHeight() - 1));

    const IV2 pointA = IV2_F24_8(fPointA);
    const IV2 pointB = IV2_F24_8(fPointB);
    const IV2 pointC = IV2_F24_8(fPointC);

    const IV2 edge0 = pointB - pointA;
    const IV2 edge1 = pointC - pointB;
    const IV2 edge2 = pointA - pointC;

    /* discard backfaces early */
    if (IV2Cross(edge0, pointC - pointA) > 0)
        return;

    const bool bTopLeft0 = (edge0.y > 0) || (edge0.x > 0 && edge0.y == 0);
    const bool bTopLeft1 = (edge1.y > 0) || (edge1.x > 0 && edge1.y == 0);
    const bool bTopLeft2 = (edge2.y > 0) || (edge2.x > 0 && edge2.y == 0);

    const simd::f32x8 barycentricDiv = 256.0f / static_cast<f32>(IV2Cross(pointB - pointA, pointC - pointA));

    simd::i32x8 edge0DiffX = edge0.y;
    simd::i32x8 edge1DiffX = edge1.y;
    simd::i32x8 edge2DiffX = edge2.y;

    const simd::i32x8 edge0DiffY = -edge0.x;
    const simd::i32x8 edge1DiffY = -edge1.x;
    const simd::i32x8 edge2DiffY = -edge2.x;

    simd::i32x8 edge0RowY {};
    simd::i32x8 edge1RowY {};
    simd::i32x8 edge2RowY {};
    {
        IV2 startPos = IV2_F24_8(V2From(minX, minY) + V2{0.5f, 0.5f});
        i64 edge0RowY64 = IV2Cross(startPos - pointA, edge0);
        i64 edge1RowY64 = IV2Cross(startPos - pointB, edge1);
        i64 edge2RowY64 = IV2Cross(startPos - pointC, edge2);

        i32 edge0RowY32 = i32((edge0RowY64 + math::sign(edge0RowY64)*128) / 256) - (bTopLeft0 ? 0 : -1);
        i32 edge1RowY32 = i32((edge1RowY64 + math::sign(edge1RowY64)*128) / 256) - (bTopLeft1 ? 0 : -1);
        i32 edge2RowY32 = i32((edge2RowY64 + math::sign(edge2RowY64)*128) / 256) - (bTopLeft2 ? 0 : -1);

        edge0RowY = simd::i32x8(edge0RowY32) + simd::i32x8(0, 1, 2, 3, 4, 5, 6, 7) * edge0DiffX;
        edge1RowY = simd::i32x8(edge1RowY32) + simd::i32x8(0, 1, 2, 3, 4, 5, 6, 7) * edge1DiffX;
        edge2RowY = simd::i32x8(edge2RowY32) + simd::i32x8(0, 1, 2, 3, 4, 5, 6, 7) * edge2DiffX;
    }

    edge0DiffX *= 8;
    edge1DiffX *= 8;
    edge2DiffX *= 8;

    for (int y = minY; y <= maxY; ++y)
    {
        simd::i32x8 edge0RowX = edge0RowY;
        simd::i32x8 edge1RowX = edge1RowY;
        simd::i32x8 edge2RowX = edge2RowY;

        for (int x = minX; x <= maxX; x += 8)
        {
            i32* pColor = reinterpret_cast<i32*>(&sp(x, y));
            f32* pDepth = &spDepth(x, y);
            simd::i32x8 pixelColors;
            simd::f32x8 pixelDepths;
            {
                /*guard::Mtx lock(&s_mtxDepth);*/
                pixelColors = simd::i32x8Load(pColor);
                pixelDepths = simd::f32x8Load(pDepth);
            }
            /*simd::i32x8 pixelColors = simd::i32x8Load(pColor);*/
            /*simd::f32x8 pixelDepths = simd::f32x8Load(pDepth);*/

            const simd::i32x8 edgeMask = (edge0RowX | edge1RowX | edge2RowX) >= 0;

            if (simd::moveMask8(edgeMask) != 0)
            {
                const simd::f32x8 t0 = -simd::f32x8(edge1RowX) * barycentricDiv;
                const simd::f32x8 t1 = -simd::f32x8(edge2RowX) * barycentricDiv;
                const simd::f32x8 t2 = -simd::f32x8(edge0RowX) * barycentricDiv;

                const simd::f32x8 depthZ = vertex0.pos.z + t1*(vertex1.pos.z - vertex0.pos.z) + t2*(vertex2.pos.z - vertex0.pos.z);
                const simd::i32x8 depthMask = simd::i32x8Reinterpret(depthZ < pixelDepths);

                const simd::f32x8 oneOverW = t0*vertex0.pos.w + t1*vertex1.pos.w + t2*vertex2.pos.w;

                simd::V2x8 uv = t0*vertex0.uv + t1*vertex1.uv + t2*vertex2.uv;
                uv /= oneOverW;

                simd::i32x8 texelColor = 0;

                switch (eSampler)
                {
                    case SAMPLER::NEAREST:
                    {
                        simd::i32x8 texelX = simd::i32x8(simd::floor(uv.x * (spTexture.getWidth() - 1)));
                        simd::i32x8 texelY = simd::i32x8(simd::floor(uv.y * (spTexture.getHeight() - 1)));

                        const simd::i32x8 texelMask = (
                            (texelX >= 0) & (texelX < spTexture.getWidth()) &
                            (texelY >= 0) & (texelY < spTexture.getHeight())
                        );

                        texelX = simd::max(simd::min(texelX, spTexture.getWidth() - 1), 0);
                        texelY = simd::max(simd::min(texelY, spTexture.getHeight() - 1), 0);
                        simd::i32x8 texelOffsets = texelY * spTexture.getWidth() + texelX;

                        const simd::i32x8 trueCase = simd::i32x8Gather((i32*)spTexture.data(), texelOffsets);
                        const simd::i32x8 falseCase = 0xff00ff00;

                        texelColor = (trueCase & texelMask) + simd::andNot(texelMask, falseCase);
                    }
                    break;

                    case SAMPLER::BILINEAR:
                    {
                        simd::V2x8 texelV2 = uv *
                            V2From(spTexture.getWidth(), spTexture.getHeight()) -
                            V2{0.5f, 0.5f};

                        simd::IV2x8 aTexelPos[4] {};
                        aTexelPos[0] = simd::IV2x8(simd::floor(texelV2.x), simd::floor(texelV2.y));
                        aTexelPos[1] = aTexelPos[0] + IV2{1, 0};
                        aTexelPos[2] = aTexelPos[0] + IV2{0, 1};
                        aTexelPos[3] = aTexelPos[0] + IV2{1, 1};

                        simd::V3x8 aTexelColors[4] {};
                        for (int texelI = 0; texelI < utils::size(aTexelPos); ++texelI)
                        {
                            simd::IV2x8 currTexelPos = aTexelPos[texelI];
                            {
                                simd::V2x8 currTexelPosF = simd::V2x8(currTexelPos);
                                simd::V2x8 factor = simd::floor(currTexelPosF / V2From(spTexture.getWidth(), spTexture.getHeight()));
                                currTexelPosF = currTexelPosF - factor * V2From(spTexture.getWidth(), spTexture.getHeight());
                                currTexelPos = simd::IV2x8(currTexelPosF);
                            }

                            simd::i32x8 texelOffsets = currTexelPos.y * spTexture.getWidth() + currTexelPos.x;
                            simd::i32x8 loadMask = edgeMask & depthMask;
                            texelOffsets = (texelOffsets & loadMask) + simd::andNot(loadMask, simd::i32x8(0));
                            simd::i32x8 texelColorI32 = simd::i32x8Gather((i32*)spTexture.data(), texelOffsets);

                            aTexelColors[texelI] = colorI32x8ToV3x8(texelColorI32);
                        }

                        simd::f32x8 s = texelV2.x - simd::floor(texelV2.x);
                        simd::f32x8 k = texelV2.y - simd::floor(texelV2.y);

                        simd::V3x8 interpolated0 = lerp(aTexelColors[0], aTexelColors[1], s);
                        simd::V3x8 interpolated1 = lerp(aTexelColors[2], aTexelColors[3], s);
                        simd::V3x8 finalColor = lerp(interpolated0, interpolated1, k);

                        texelColor = colorV3x8ToI32x8(finalColor);
                    }
                    break;
                }

                const simd::i32x8 finalMaskI32 = edgeMask & depthMask;
                const simd::f32x8 finalMaskF32 = simd::f32x8Reinterpret(finalMaskI32);
                const simd::i32x8 outputColors = (texelColor & finalMaskI32) + simd::andNot(finalMaskI32, pixelColors);
                const simd::f32x8 outputDepth = (depthZ & finalMaskF32) + simd::andNot(finalMaskF32, pixelDepths);

                {
                    /*guard::Mtx lock(&s_mtxDepth);*/
                    simd::i32x8Store(pColor, outputColors);
                    simd::f32x8Store(pDepth, outputDepth);
                }
            }
            edge0RowX += edge0DiffX;
            edge1RowX += edge1DiffX;
            edge2RowX += edge2DiffX;
        }
        edge0RowY += edge0DiffY;
        edge1RowY += edge1DiffY;
        edge2RowY += edge2DiffY;
    }
}

#endif

ADT_NO_UB static void
drawTriangle(
    const math::V4 p0, const math::V4 p1, const math::V4 p2,
    const math::V2 uv0, const math::V2 uv1, const math::V2 uv2,
    const Span2D<ImagePixelRGBA> spTexture
)
{
    clip::Result ping {};
    ping.nTriangles = 1;
    ping.aVertices[0] = {p0, uv0};
    ping.aVertices[1] = {p1, uv1};
    ping.aVertices[2] = {p2, uv2};

    clip::Result pong {};

    clip::polygonToAxis(&ping, &pong, clip::AXIS::LEFT);
    clip::polygonToAxis(&pong, &ping, clip::AXIS::RIGHT);
    clip::polygonToAxis(&ping, &pong, clip::AXIS::TOP);
    clip::polygonToAxis(&pong, &ping, clip::AXIS::BOTTOM);
    clip::polygonToAxis(&ping, &pong, clip::AXIS::NEAR);
    clip::polygonToAxis(&pong, &ping, clip::AXIS::FAR);
    clip::polygonToAxis(&ping, &pong, clip::AXIS::W);

    for (int triangleIdx = 0; triangleIdx < pong.nTriangles; ++triangleIdx)
    {
        auto v0 = pong.aVertices[3*triangleIdx + 0];
        auto v1 = pong.aVertices[3*triangleIdx + 1];
        auto v2 = pong.aVertices[3*triangleIdx + 2];

#ifdef ADT_AVX2
        drawTriangleAVX2(v0, v1, v2, spTexture, SAMPLER::BILINEAR);
#else
        drawTriangleSSE(v0, v1, v2, spTexture, SAMPLER::BILINEAR);
#endif /* ADT_AVX2 */
    }
}

[[maybe_unused]] static void
helloGradientTest()
{
    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();

    static int frame = 0;

    for (ssize y = 0; y < sp.getHeight(); ++y)
    {
        for (ssize x = 0; x < sp.getWidth(); ++x)
        {
            ImagePixelRGBA pix;
            pix.r = 255;
            pix.g = static_cast<u8>(x);
            pix.b = static_cast<u8>(x + frame);
            pix.a = static_cast<u8>(y - frame);

            sp(x, y).data = pix.data;
        }
    }

    ++frame;
}

static void
drawGLTFNode(Arena* pArena, gltf::Model& model, gltf::Node& node, math::M4 trm)
{
    using namespace adt::math;

    /* NOTE: must be one of asset::g_objects */
    auto pObj = (const asset::Object*)&model;

    common::updateModelNode(&model, &node, &trm);

    for (auto& children : node.vChildren)
    {
        auto& childNode = model.m_vNodes[children];
        drawGLTFNode(pArena, model, childNode, trm);
    }

    if (node.meshI != NPOS)
    {
        auto& mesh = model.m_vMeshes[node.meshI];
        for (const auto& primitive : mesh.vPrimitives)
        {
            /* TODO: can be NPOS */
            ADT_ASSERT(primitive.indicesI != -1, " ");

            auto& accIndices = model.m_vAccessors[primitive.indicesI];
            auto& viewIndicies = model.m_vBufferViews[accIndices.bufferViewI];
            auto& buffInd = model.m_vBuffers[viewIndicies.bufferI];

            /* TODO: there might be any number of TEXCOORD_*,
             * which would be specified in baseColorTexture.texCoord.
             * But current gltf parser only reads the 0th. */
            gltf::Accessor accUV {};
            gltf::BufferView viewUV {};
            gltf::Buffer buffUV {};
            if (primitive.attributes.TEXCOORD_0 != -1)
            {
                accUV = model.m_vAccessors[primitive.attributes.TEXCOORD_0];
                viewUV = model.m_vBufferViews[accUV.bufferViewI];
                buffUV = model.m_vBuffers[viewUV.bufferI];
            }

            auto& accPos = model.m_vAccessors[primitive.attributes.POSITION];
            auto& viewPos = model.m_vBufferViews[accPos.bufferViewI];
            auto& buffPos = model.m_vBuffers[viewPos.bufferI];

            Span2D<ImagePixelRGBA> spImage = common::g_spDefaultTexture;
            if (primitive.materialI != -1)
            {
                gltf::Material mat = model.m_vMaterials[primitive.materialI];
                int baseTextureIdx = mat.pbrMetallicRoughness.baseColorTexture.index;
                if (baseTextureIdx != static_cast<i32>(NPOS))
                {
                    auto& imgIdx = model.m_vTextures[baseTextureIdx].sourceI;
                    auto& uri = model.m_vImages[imgIdx].sUri;

                    String nPath = file::replacePathEnding(pArena, pObj->m_sMappedWith, uri);
                    Image* pImg = asset::searchImage(nPath);

                    if (pImg)
                        spImage = pImg->getSpanRGBA();
                }
            }

            ADT_ASSERT(accIndices.eComponentType == gltf::COMPONENT_TYPE::UNSIGNED_SHORT ||
                accIndices.eComponentType == gltf::COMPONENT_TYPE::UNSIGNED_INT,
                "exp: %d or %d, got: %d",
                (int)gltf::COMPONENT_TYPE::UNSIGNED_SHORT,
                (int)gltf::COMPONENT_TYPE::UNSIGNED_INT,
                (int)accIndices.eComponentType
            );

            Span<V2> spUVs {};
            if (primitive.attributes.TEXCOORD_0 != -1)
            {
                spUVs = {
                    (V2*)&buffUV.sBin[accUV.byteOffset + viewUV.byteOffset],
                    accUV.count
                };
            }

            ADT_ASSERT(accPos.eType == gltf::Accessor::TYPE::VEC3, " ");
            const Span<V3> spPos {
                (V3*)&buffPos.sBin[accPos.byteOffset + viewPos.byteOffset],
                accPos.count
            };

            switch (primitive.eMode)
            {
                default: break;

                case gltf::Primitive::TYPE::TRIANGLES:
                {
                    if (accIndices.count < 3)
                    {
                        LOG_BAD("accIndices.count: {}\n", accIndices.count);
                        return;
                    }

                    switch (accIndices.eComponentType)
                    {
                        default: break;

                        case gltf::COMPONENT_TYPE::UNSIGNED_SHORT:
                        {
                            const Span<IndexU16x3> spIndiciesU16 {
                                (IndexU16x3*)&buffInd.sBin[accIndices.byteOffset + viewIndicies.byteOffset],
                                accIndices.count / 3
                            };

                            for (const auto [i0, i1, i2] : spIndiciesU16)
                            {
                                V3 aPos[3] {spPos[i0], spPos[i1], spPos[i2]};
                                V2 aUVs[3] {};
                                if (primitive.attributes.TEXCOORD_0 != -1)
                                {
                                    aUVs[0] = spUVs[i0];
                                    aUVs[1] = spUVs[i1];
                                    aUVs[2] = spUVs[i2];
                                }

                                drawTriangle(
                                    trm*V4From(aPos[0], 1.0f), trm * V4From(aPos[1], 1.0f), trm* V4From(aPos[2], 1.0f),
                                    aUVs[0], aUVs[1], aUVs[2],
                                    spImage
                                );
                            }
                        }
                        break;

                        case gltf::COMPONENT_TYPE::UNSIGNED_INT:
                        {
                            const Span<IndexU32x3> spIndiciesU32 {
                                (IndexU32x3*)&buffInd.sBin[accIndices.byteOffset + viewIndicies.byteOffset],
                                accIndices.count / 3
                            };

                            for (const auto [i0, i1, i2] : spIndiciesU32)
                            {
                                V3 aPos[3] {spPos[i0], spPos[i1], spPos[i2]};
                                V2 aUVs[3] {};
                                if (primitive.attributes.TEXCOORD_0 != -1)
                                {
                                    aUVs[0] = spUVs[i0];
                                    aUVs[1] = spUVs[i1];
                                    aUVs[2] = spUVs[i2];
                                }

                                drawTriangle(
                                    trm*V4From(aPos[0], 1.0f), trm*V4From(aPos[1], 1.0f), trm*V4From(aPos[2], 1.0f),
                                    aUVs[0], aUVs[1], aUVs[2],
                                    spImage
                                );
                            }
                        }
                        break;
                    } /* switch (accIndices.componentType) */
                }
                break;
            } /* switch (primitive.mode) */
        }
    }
}

static void
drawGLTF(Arena* pArena, gltf::Model& model, math::M4 trm)
{
    auto& scene = model.m_vScenes[model.m_defaultScene.nodeI];
    for (auto& nodeI : scene.vNodes)
        drawGLTFNode(pArena, model, model.m_vNodes[nodeI], trm);
}

[[maybe_unused]] static void
drawImgDBG(Image* pImg)
{
    ADT_ASSERT(pImg, "pImg == nullptr");

    auto& win = app::window();
    auto sp = win.surfaceBuffer();
    auto spImg = pImg->getSpanRGBA();

    const f32 xStep = static_cast<f32>(spImg.getWidth()) / static_cast<f32>(sp.getWidth());
    const f32 yStep = static_cast<f32>(spImg.getHeight()) / static_cast<f32>(sp.getHeight());

    for (int y = 0; y < sp.getHeight(); ++y)
    {
        for (int x = 0; x < sp.getWidth(); ++x)
            sp(x, y) = spImg(x * xStep, y * yStep);
    }
}

void
Renderer::init()
{
}

void
Renderer::drawEntities(Arena* pArena)
{
    using namespace adt::math;

    static Vec<f64> s_vFrameTimes(OsAllocatorGet(), 1000);
    static f64 s_lastAvgFrameTimeUpdate {};

    const f64 t0 = utils::timeNowMS();

    auto& win = app::window();

    win.clearSurfaceBuffer({0.1f, 0.1f, 0.1f, 1.0f});
    win.clearDepthBuffer();

    const auto& camera = control::g_camera;
    const f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);

    {
        for (int entityI = 0; entityI < game::g_poolEntites.m_size; ++entityI)
        {
            const auto& arrays = game::g_poolEntites.m_arrays;
            if (arrays.abDead[entityI] || arrays.priv.abFree[entityI])
                continue;

            game::EntityBind entity = game::g_aEntites[ game::Entity{.i = entityI} ];

            M4 trm = M4Pers(toRad(camera.m_fov), aspectRatio, 0.01f, 1000.0f) *
                camera.m_trm *
                M4TranslationFrom(entity.pos) *
                QtRot(entity.rot) *
                M4ScaleFrom(entity.scale);

            auto& obj = asset::g_poolObjects[entity.assetI];
            switch (obj.m_eType)
            {
                default: break;

                case asset::Object::TYPE::MODEL:
                {
                    drawGLTF(pArena, obj.m_uData.model, trm);
                }
                break;
            }
        }
    }

    const f64 t1 = utils::timeNowMS();
    s_vFrameTimes.push(t1 - t0);

    if (t1 > s_lastAvgFrameTimeUpdate + 1000.0)
    {
        f64 avg = 0;
        for (f64 ft : s_vFrameTimes) avg += ft;
        CERR("FPS: {} | avg frame time: {} ms\n", s_vFrameTimes.size(), avg / s_vFrameTimes.size());
        s_vFrameTimes.setSize(0);
        s_lastAvgFrameTimeUpdate = t1;
    }
}

} /* namespace render::sw */
