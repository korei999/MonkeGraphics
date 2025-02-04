/* Rasterization goes here */

#include "adt/OsAllocator.hh"
#include "adt/logs.hh"
#include "adt/math.hh"
#include "adt/ScratchBuffer.hh"

#include "app.hh"
#include "asset.hh"
#include "clip.hh"
#include "control.hh"
#include "draw.hh"
#include "frame.hh"

using namespace adt;

namespace draw
{

struct Idx3 { u16 x, y, z; };

u8 aScratchMem[SIZE_8K] {};
ScratchBuffer s_scratch {aScratchMem};

[[maybe_unused]] static Span2D<ImagePixelARGB>
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

                simd::i32x4 texelColor = 0;

                /* nearest neighbor filtering */
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
helloGLTF()
{
    using namespace adt::math;

    static Span2D<ImagePixelARGB> spDefaultTexture = allocDefaultTexture();

    const auto& win = app::window();
    const auto& model = *asset::searchModel("assets/Duck.gltf");
    const auto& camera = control::g_camera;
    const f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);
    const f32 step = static_cast<f32>(frame::g_time*0.0010);

    M4 tr = M4Pers(toRad(60.0f), aspectRatio, 0.01f, 1000.0f) *
        camera.m_trm *
        M4TranslationFrom(0.0f, 0.0f, -1.0f) *
        M4RotFrom(0, step, 0) *
        M4ScaleFrom(0.01f);

    for (const auto& node : model.m_aNodes)
    {
        if (node.mesh == NPOS)
            continue;

        auto& mesh = model.m_aMeshes[node.mesh];
        for (auto& primitive : mesh.aPrimitives)
        {
            /* TODO: can be NPOS32 */
            ADT_ASSERT(primitive.indices != static_cast<i32>(NPOS32), " ");
            auto& accIndices = model.m_aAccessors[primitive.indices];
            auto& viewIndicies = model.m_aBufferViews[accIndices.bufferView];
            auto& buffInd = model.m_aBuffers[viewIndicies.buffer];

            auto& accUV = model.m_aAccessors[primitive.attributes.TEXCOORD_0];
            auto& viewUV = model.m_aBufferViews[accUV.bufferView];
            auto& buffUV = model.m_aBuffers[viewUV.buffer];

            auto& accPos = model.m_aAccessors[primitive.attributes.POSITION];
            auto& viewPos = model.m_aBufferViews[accPos.bufferView];
            auto& buffPos = model.m_aBuffers[viewPos.buffer];

            Span2D<ImagePixelARGB> spImage = spDefaultTexture;
            auto& mat = model.m_aMaterials[primitive.material];
            auto& baseTextureIdx = mat.pbrMetallicRoughness.baseColorTexture.index;
            if (baseTextureIdx != static_cast<i32>(NPOS))
            {
                auto& imgIdx = model.m_aTextures[baseTextureIdx].source;
                auto& uri = model.m_aImages[imgIdx].uri;
                char aBuff[512] {};
                ssize n = print::toSpan(aBuff, "assets/{}", uri);
                Image* pImg = asset::searchImage({aBuff, n});
                if (pImg)
                    spImage = pImg->getSpanARGB();
            }

            /* TODO: support every possible component type */

            ADT_ASSERT(accIndices.componentType == gltf::COMPONENT_TYPE::UNSIGNED_SHORT, " ");
            const Span<Idx3> spIndicies {
                (Idx3*)&buffInd.bin[accIndices.byteOffset + viewIndicies.byteOffset],
                accIndices.count / 3
            };

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

                    for (auto [i0, i1, i2] : spIndicies)
                    {
                        V3 aPos[3] {spPos[i0], spPos[i1], spPos[i2]};
                        V2 aUVs[3] {spUVs[i0], spUVs[i1], spUVs[i2]};

                        drawTriangle(
                            tr * V4From(aPos[0], 1.0f), tr * V4From(aPos[1], 1.0f), tr* V4From(aPos[2], 1.0f),
                            aUVs[0], aUVs[1], aUVs[1],
                            spImage
                        );
                    }
                }
                break;
            }
        }
    }
}

void
toBuffer()
{
    auto& win = app::window();

    static Vec<f32> s_vCollect(OsAllocatorGet(), 1000);
    static f64 s_lastCollectionUpdate {};

    f64 t0 = utils::timeNowMS();

    /* clear */
    win.clearBuffer();
    win.clearDepthBuffer();

    helloGLTF();

    f64 t1 = utils::timeNowMS();
    s_vCollect.push(t1 - t0);

    if (t1 > s_lastCollectionUpdate + 1000.0)
    {
        f64 avg = 0;
        for (auto ft : s_vCollect) avg += ft;
        CERR("avg frame time: {} ms\n", avg / s_vCollect.getSize());
        s_vCollect.setSize(0);
        s_lastCollectionUpdate = t1;
    }
}

} /* namespace draw */
