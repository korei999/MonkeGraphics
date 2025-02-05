/* Rasterization goes here */

#include "adt/OsAllocator.hh"
#include "adt/logs.hh"
#include "adt/math.hh"
#include "adt/ScratchBuffer.hh"
#include "adt/file.hh"

#include "app.hh"
#include "asset.hh"
#include "clip.hh"
#include "control.hh"
#include "draw.hh"
#include "frame.hh"

using namespace adt;

namespace draw
{

enum SAMPLER : u8 { NEAREST, BILINEAR };

struct IdxU16x3 { u16 x, y, z; };
struct IdxU32x3 { u32 x, y, z; };

static u8 s_aScratchMem[SIZE_8K] {};
static ScratchBuffer s_scratch {s_aScratchMem};
static Span2D<ImagePixelARGB> s_spDefaultTexture;

static CallOnce s_callOnceAllocDefaultTexture(INIT);

Span2D<ImagePixelARGB>
allocDefaultTexture()
{
    const int width = 8;
    const int height = 8;

    static ImagePixelARGB aPixels[width * height] {};

    Span2D sp(aPixels, width, height, width);
    for (int y = 0; y < sp.getHeight(); ++y)
    {
        for (int x = 0; x < sp.getWidth(); ++x)
        {
            u32 colorChannel = 255 * ((x + (y % 2)) % 2);
            ImagePixelARGB p {
                .b = static_cast<u8>(colorChannel),
                .g = static_cast<u8>(colorChannel),
                .r = static_cast<u8>(0),
                .a = 255,
            };
            sp(x, y) = p;

            // sp(x, y).data = -1;
        }
    }

    return sp;
}

static simd::V3x4
colorI32x4ToV3x4(simd::i32x4 color)
{
    simd::V3x4 res;
    res.x = simd::f32x4((color >> 16) & 0xFF);
    res.y = simd::f32x4((color >> 8) & 0xFF);
    res.z = simd::f32x4((color >> 0) & 0xFF);
    res = res / 255.0f;
    return res;
}

static simd::i32x4
colorV3x4ToI32x4(simd::V3x4 color)
{
    color *= 255.0f;
    simd::i32x4 res = ((simd::i32x4(0xff) << 24) |
        (simd::i32x4(color.x) << 16) |
        (simd::i32x4(color.y) << 8) |
        (simd::i32x4(color.z))
    );

    return res;
}

static math::V2
ndcToPix(math::V2 ndcPos)
{
    using namespace adt::math;
    const auto& win = *app::g_pWindow;

    V2 res = 0.5f * (ndcPos + V2{1.0f, 1.0f});
    res *= V2{static_cast<f32>(win.m_width), static_cast<f32>(win.m_height)};

    return res;
}

[[maybe_unused]] ADT_NO_UB static void
drawTriangleSSE(
    clip::Vertex vertex0, clip::Vertex vertex1, clip::Vertex vertex2,
    const Span2D<ImagePixelARGB> spTexture
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
            const int invY = sp.getHeight() - 1 - y;
            i32* pColor = &sp(x, invY).iData;
            f32* pDepth = &spDepth(x, invY);
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

                /* nearest neighbor filtering */
                // {
                //     simd::i32x4 texelX = simd::i32x4(simd::floor(uv.x * (spTexture.getWidth() - 1)));
                //     simd::i32x4 texelY = simd::i32x4(simd::floor(uv.y * (spTexture.getHeight() - 1)));

                //     const simd::i32x4 texelMask = (
                //         (texelX >= 0) & (texelX < spTexture.getWidth()) &
                //         (texelY >= 0) & (texelY < spTexture.getHeight())
                //     );

                //     texelX = simd::max(simd::min(texelX, spTexture.getWidth() - 1), 0);
                //     texelY = simd::max(simd::min(texelY, spTexture.getHeight() - 1), 0);
                //     simd::i32x4 texelOffsets = texelY * spTexture.getWidth() + texelX;

                //     const simd::i32x4 trueCase = simd::i32x4Gather((i32*)spTexture.data(), texelOffsets);
                //     const simd::i32x4 falseCase = 0xff00ff00;

                //     texelColor = (trueCase & texelMask) + simd::andNot(texelMask, falseCase);
                // }

                /* bilinear */
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
    const Span2D<ImagePixelARGB> spTexture
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
            const int invY = sp.getHeight() - 1 - y;
            i32* pColor = reinterpret_cast<i32*>(&sp(x, invY));
            f32* pDepth = &spDepth(x, invY);
            const simd::i32x8 pixelColors = simd::i32x8Load(pColor);
            const simd::f32x8 pixelDepths = simd::f32x8Load(pDepth);

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

                /* nearest neighbor filtering */
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

                    texelColor = (texelMask & trueCase) + simd::andNot(texelMask, falseCase);
                }

                const simd::i32x8 finalMaskI32 = edgeMask & depthMask;
                const simd::f32x8 finalMaskF32 = simd::f32x8Reinterpret(finalMaskI32);
                const simd::i32x8 outputColors = (texelColor & finalMaskI32) + simd::andNot(finalMaskI32, pixelColors);
                const simd::f32x8 outputDepth = (depthZ & finalMaskF32) + simd::andNot(finalMaskF32, pixelDepths);

                simd::i32x8Store(pColor, outputColors);
                simd::f32x8Store(pDepth, outputDepth);
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
    const Span2D<ImagePixelARGB> spTexture
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
        drawTriangleAVX2(v0, v1, v2, spTexture);
#else
        drawTriangleSSE(v0, v1, v2, spTexture);
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
            ImagePixelARGB pix {
                .b = static_cast<u8>(x),
                .g = static_cast<u8>(x + frame),
                .r = static_cast<u8>(y - frame),
                .a = 255,
            };

            sp(x, y).data = pix.data;
        }
    }

    ++frame;
}

static void
drawGLTFNode(Arena* pArena, const gltf::Model& model, const gltf::Node& node, math::M4 trm)
{
    using namespace adt::math;

    /* NOTE: must be from the asset::g_objects, maybe pass Object* instead? */
    const asset::Object* pObj = (asset::Object*)&model;

    trm *= node.matrix;

    for (auto& children : node.children)
    {
        auto& childNode = model.m_vNodes[children];
        drawGLTFNode(pArena, model, childNode, trm);
    }

    if (node.mesh != NPOS)
    {
        auto& mesh = model.m_vMeshes[node.mesh];
        for (auto& primitive : mesh.aPrimitives)
        {
            /* TODO: can be NPOS */
            ADT_ASSERT(primitive.indices != static_cast<i32>(NPOS32), " ");

            auto& accIndices = model.m_vAccessors[primitive.indices];
            auto& viewIndicies = model.m_vBufferViews[accIndices.bufferView];
            auto& buffInd = model.m_vBuffers[viewIndicies.buffer];

            auto& accUV = model.m_vAccessors[primitive.attributes.TEXCOORD_0];
            auto& viewUV = model.m_vBufferViews[accUV.bufferView];
            auto& buffUV = model.m_vBuffers[viewUV.buffer];

            auto& accPos = model.m_vAccessors[primitive.attributes.POSITION];
            auto& viewPos = model.m_vBufferViews[accPos.bufferView];
            auto& buffPos = model.m_vBuffers[viewPos.buffer];

            Span2D<ImagePixelARGB> spImage = s_spDefaultTexture;
            auto& mat = model.m_vMaterials[primitive.material];
            auto& baseTextureIdx = mat.pbrMetallicRoughness.baseColorTexture.index;
            if (baseTextureIdx != static_cast<i32>(NPOS))
            {
                auto& imgIdx = model.m_vTextures[baseTextureIdx].source;
                auto& uri = model.m_vImages[imgIdx].uri;

                String nPath = file::replacePathEnding(pArena, pObj->m_sMappedWith, uri);
                Image* pImg = asset::searchImage(nPath);

                if (pImg)
                    spImage = pImg->getSpanARGB();
            }

            /* TODO: support every possible component type */

            ADT_ASSERT(accIndices.componentType == gltf::COMPONENT_TYPE::UNSIGNED_SHORT ||
                accIndices.componentType == gltf::COMPONENT_TYPE::UNSIGNED_INT,
                "exp: %d or %d, got: %d",
                (int)gltf::COMPONENT_TYPE::UNSIGNED_SHORT,
                (int)gltf::COMPONENT_TYPE::UNSIGNED_INT,
                (int)accIndices.componentType
            );

            ADT_ASSERT(accUV.type == gltf::ACCESSOR_TYPE::VEC2, " ");
            const Span<V2> spUVs {
                (V2*)&buffUV.bin[accUV.byteOffset + viewUV.byteOffset],
                accUV.count
            };

            ADT_ASSERT(accPos.type == gltf::ACCESSOR_TYPE::VEC3, " ");
            const Span<V3> spPos {
                (V3*)&buffPos.bin[accPos.byteOffset + viewPos.byteOffset],
                accPos.count
            };

            ADT_ASSERT(accUV.count == accPos.count, " ");

            switch (primitive.mode)
            {
                default: break;

                case gltf::PRIMITIVES::TRIANGLES:
                {
                    if (accIndices.count < 3)
                    {
                        LOG_WARN("accIndices.count: {}\n", accIndices.count);
                        return;
                    }

                    switch (accIndices.componentType)
                    {
                        default: break;

                        case gltf::COMPONENT_TYPE::UNSIGNED_SHORT:
                        {
                            const Span<IdxU16x3> spIndiciesU16 {
                                (IdxU16x3*)&buffInd.bin[accIndices.byteOffset + viewIndicies.byteOffset],
                                accIndices.count / 3
                            };

                            //for (auto [i0, i1, i2] : spIndiciesU16)
                            #pragma omp parallel for
                            for (int i = 0; i < spIndiciesU16.getSize(); ++i)
                            {
                                int i0 = spIndiciesU16[i].x;
                                int i1 = spIndiciesU16[i].y;
                                int i2 = spIndiciesU16[i].z;

                                V3 aPos[3] {spPos[i0], spPos[i1], spPos[i2]};
                                V2 aUVs[3] {spUVs[i0], spUVs[i1], spUVs[i2]};

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
                            const Span<IdxU32x3> spIndiciesU32 {
                                (IdxU32x3*)&buffInd.bin[accIndices.byteOffset + viewIndicies.byteOffset],
                                accIndices.count / 3
                            };

                            // for (auto [i0, i1, i2] : spIndiciesU32)
                            #pragma omp parallel for
                            for (int i = 0; i < spIndiciesU32.getSize(); ++i)
                            {
                                int i0 = spIndiciesU32[i].x;
                                int i1 = spIndiciesU32[i].y;
                                int i2 = spIndiciesU32[i].z;

                                V3 aPos[3] {spPos[i0], spPos[i1], spPos[i2]};
                                V2 aUVs[3] {spUVs[i0], spUVs[i1], spUVs[i2]};

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
drawGLTF(Arena* pArena, const gltf::Model& model, math::M4 trm)
{
    for (auto& scene : model.m_vScenes)
    {
        auto& node = model.m_vNodes[scene.nodeIdx];
        drawGLTFNode(pArena, model, node, trm);
    }
}

static void
drawImgDBG(Image* pImg)
{
    auto& win = app::window();
    auto sp = win.surfaceBuffer();
    auto spImg = pImg->getSpanARGB();

    const f32 xStep = static_cast<f32>(spImg.getWidth()) / static_cast<f32>(sp.getWidth());
    const f32 yStep = static_cast<f32>(spImg.getHeight()) / static_cast<f32>(sp.getHeight());

    for (int y = 0; y < sp.getHeight(); ++y)
    {
        for (int x = 0; x < sp.getWidth(); ++x)
        {
            const int invY = sp.getHeight() - 1 - y;

            sp(x, y) = spImg(x * xStep, invY * yStep);
        }
    }

}

void
toBuffer(Arena* pArena)
{
    using namespace adt::math;

    auto& win = app::window();

    static Vec<f64> s_vFrameTimes(OsAllocatorGet(), 1000);
    static f64 s_lastAvgFrameTimeUpdate {};

    const f64 t0 = utils::timeNowMS();

    s_callOnceAllocDefaultTexture.exec(
        +[] { s_spDefaultTexture = allocDefaultTexture(); }
    );

    {
        /* clear */
        win.clearColorBuffer({0.0f, 0.4f, 0.6f, 1.0f});
        win.clearDepthBuffer();

        const auto& model = *asset::searchModel("assets/Sponza/Sponza.gltf");
        const auto& camera = control::g_camera;
        const f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);
        const f32 step = static_cast<f32>(frame::g_time*0.001);

        M4 cameraTrm = M4Pers(toRad(60.0f), aspectRatio, 0.01f, 1000.0f) *
            camera.m_trm *
            M4TranslationFrom(0.0f, -0.0f, -0.0f) *
            M4RotFrom(0, 0, 0) *
            M4ScaleFrom(0.006f);

        drawGLTF(pArena, model, cameraTrm);
    }

    const f64 t1 = utils::timeNowMS();
    s_vFrameTimes.push(t1 - t0);

    if (t1 > s_lastAvgFrameTimeUpdate + 1000.0)
    {
        f64 avg = 0;
        for (f64 ft : s_vFrameTimes) avg += ft;
        CERR("avg frame time: {} ms, nSamples: {}\n", avg / s_vFrameTimes.getSize(), s_vFrameTimes.getSize());
        s_vFrameTimes.setSize(0);
        s_lastAvgFrameTimeUpdate = t1;
    }
}

} /* namespace draw */
