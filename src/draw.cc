/* Rasterization goes here */

#include "adt/logs.hh"
#include "adt/math.hh"
#include "adt/ScratchBuffer.hh"

#include "app.hh"
#include "clip.hh"
#include "control.hh"
#include "draw.hh"
#include "frame.hh"

using namespace adt;

namespace draw
{

u8 aScratchMem[SIZE_8K] {};
ScratchBuffer s_scratch {aScratchMem};

static Span2D<Pixel>
allocCheckerBoardTexture()
{
    const int width = 8;
    const int height = 8;

    static Pixel aPixels[width * height] {};

    Span2D sp(aPixels, width, height);
    for (int y = 0; y < sp.getHeight(); ++y)
    {
        for (int x = 0; x < sp.getWidth(); ++x)
        {
            u32 colorChannel = 255 * ((x + (y % 2)) % 2);
            Pixel p {
                static_cast<u8>(colorChannel),
                static_cast<u8>(colorChannel),
                static_cast<u8>(colorChannel),
                255
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

ADT_NO_UB static void
drawTriangle(
    clip::Vertex vertex0, clip::Vertex vertex1, clip::Vertex vertex2,
    const Span2D<Pixel> spTexture
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

    const V2 pointA = ndcToPix(vertex0.pos.xy);
    const V2 pointB = ndcToPix(vertex1.pos.xy);
    const V2 pointC = ndcToPix(vertex2.pos.xy);

    const V2 edge0 = pointB - pointA;
    const V2 edge1 = pointC - pointB;
    const V2 edge2 = pointA - pointC;

    /* discard backfaces early */
    if (V2Cross(edge0, pointC - pointA) > 0)
        return;

    int minX = utils::min(
        utils::min(static_cast<int>(pointA.x), static_cast<int>(pointB.x)),
        static_cast<int>(pointC.x)
    );
    int maxX = utils::max(
        utils::max(static_cast<int>(std::round(pointA.x)), static_cast<int>(std::round(pointB.x))),
        static_cast<int>(std::round(pointC.x))
    );
    int minY = utils::min(
        utils::min(static_cast<int>(pointA.y), static_cast<int>(pointB.y)),
        static_cast<int>(pointC.y)
    );
    int maxY = utils::max(
        utils::max(static_cast<int>(std::round(pointA.y)), static_cast<int>(std::round(pointB.y))),
        static_cast<int>(std::round(pointC.y))
    );

    minX = utils::clamp(minX, 0, static_cast<int>(sp.getWidth() - 1));
    maxX = utils::clamp(maxX, 0, static_cast<int>(sp.getWidth() - 1));
    minY = utils::clamp(minY, 0, static_cast<int>(sp.getHeight() - 1));
    maxY = utils::clamp(maxY, 0, static_cast<int>(sp.getHeight() - 1));

    const bool bTopLeft0 = (edge0.y > 0.0f) || (edge0.x > 0.0f && edge0.y == 0.0f);
    const bool bTopLeft1 = (edge1.y > 0.0f) || (edge1.x > 0.0f && edge1.y == 0.0f);
    const bool bTopLeft2 = (edge2.y > 0.0f) || (edge2.x > 0.0f && edge2.y == 0.0f);

    const f32 barycentricDiv = V2Cross(pointB - pointA, pointC - pointA);
    const f32 barycentricDivInv = 1.0f / barycentricDiv;

    vertex0.uv *= vertex0.pos.w;
    vertex1.uv *= vertex1.pos.w;
    vertex2.uv *= vertex2.pos.w;

    f32 edge0DiffX = edge0.y;
    f32 edge1DiffX = edge1.y;
    f32 edge2DiffX = edge2.y;

    f32 edge0DiffY = -edge0.x;
    f32 edge1DiffY = -edge1.x;
    f32 edge2DiffY = -edge2.x;

    V2 startPos = V2From(minX, minY) + V2{0.5f, 0.5f};
    f32 edge0RowY = V2Cross(startPos - pointA, edge0);
    f32 edge1RowY = V2Cross(startPos - pointB, edge1);
    f32 edge2RowY = V2Cross(startPos - pointC, edge2);

    for (int y = minY; y <= maxY; ++y)
    {
        f32 edge0RowX = edge0RowY;
        f32 edge1RowX = edge1RowY;
        f32 edge2RowX = edge2RowY;

        for (int x = minX; x <= maxX; ++x)
        {
            /* inside triangle */
            if ((edge0RowX > 0.0f || (bTopLeft0 && edge0RowX == 0.0f)) &&
                (edge1RowX > 0.0f || (bTopLeft1 && edge1RowX == 0.0f)) &&
                (edge2RowX > 0.0f || (bTopLeft2 && edge2RowX == 0.0f))
            )
            {
                const ssize invY = sp.getHeight() - 1 - y;

                const f32 t0 = -edge1RowX * barycentricDivInv;
                const f32 t1 = -edge2RowX * barycentricDivInv;
                const f32 t2 = -edge0RowX * barycentricDivInv;

                const f32 depth = t0*vertex0.pos.z + t1*vertex1.pos.z + t2*vertex2.pos.z;
                ADT_ASSERT(depth >= 0.0f, "depth: %g", depth);
                if (depth <= 1.0f && depth < spDepth(x, invY))
                {
                    const f32 invDepthW = t0*vertex0.pos.w + t1*vertex1.pos.w + t2*vertex2.pos.w;
                    const V2 uv = (t0*vertex0.uv + t1*vertex1.uv + t2*vertex2.uv) / invDepthW;

                    const int texelX = static_cast<int>(uv.x * (spTexture.getWidth() - 1));
                    const int texelY = static_cast<int>(uv.y * (spTexture.getHeight() - 1));

                    if (texelX >= 0 && texelX < spTexture.getWidth() &&
                        texelY >= 0 && texelY < spTexture.getHeight()
                    )
                    {
                        sp(x, invY) = spTexture(texelX, texelY);
                    }
                    else
                    {
                        sp(x, invY).data = 0xffff00ff;
                    }

                    spDepth(x, invY) = depth;
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

ADT_NO_UB static void
drawTriangle2(
    clip::Vertex vertex0, clip::Vertex vertex1, clip::Vertex vertex2,
    const Span2D<Pixel> spTexture
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

    const f32 barycentricDiv = static_cast<f32>(IV2Cross(pointB - pointA, pointC - pointA)) / 256.0f;
    const f32 barycentricDivInv = 1.0f / barycentricDiv;

    const i32 edge0DiffX = edge0.y;
    const i32 edge1DiffX = edge1.y;
    const i32 edge2DiffX = edge2.y;

    const i32 edge0DiffY = -edge0.x;
    const i32 edge1DiffY = -edge1.x;
    const i32 edge2DiffY = -edge2.x;

    IV2 startPos = IV2_F24_8(V2From(minX, minY) + V2{0.5f, 0.5f});
    const i64 edge0RowY64 = IV2Cross(startPos - pointA, edge0);
    const i64 edge1RowY64 = IV2Cross(startPos - pointB, edge1);
    const i64 edge2RowY64 = IV2Cross(startPos - pointC, edge2);

    i32 edge0RowY = static_cast<i32>((edge0RowY64 + math::sign(edge0RowY64)*128) / 256) - (bTopLeft0 ? 0 : -1);
    i32 edge1RowY = static_cast<i32>((edge1RowY64 + math::sign(edge1RowY64)*128) / 256) - (bTopLeft1 ? 0 : -1);
    i32 edge2RowY = static_cast<i32>((edge2RowY64 + math::sign(edge2RowY64)*128) / 256) - (bTopLeft2 ? 0 : -1);

    for (int y = minY; y <= maxY; ++y)
    {
        i32 edge0RowX = edge0RowY;
        i32 edge1RowX = edge1RowY;
        i32 edge2RowX = edge2RowY;

        for (int x = minX; x <= maxX; ++x)
        {
            /* inside triangle */
            if (edge0RowX >= 0 && edge1RowX >= 0 && edge2RowX >= 0)
            {
                const ssize invY = sp.getHeight() - 1 - y;

                const f32 t0 = -static_cast<f32>(edge1RowX) * barycentricDivInv;
                const f32 t1 = -static_cast<f32>(edge2RowX) * barycentricDivInv;
                const f32 t2 = -static_cast<f32>(edge0RowX) * barycentricDivInv;

                const f32 depth = t0*vertex0.pos.z + t1*vertex1.pos.z + t2*vertex2.pos.z;
                ADT_ASSERT(depth >= 0.0f, "depth: %g", depth);
                if (depth <= 1.0f && depth < spDepth(x, invY))
                {
                    const f32 invDepthW = t0*vertex0.pos.w + t1*vertex1.pos.w + t2*vertex2.pos.w;
                    const V2 uv = (t0*vertex0.uv + t1*vertex1.uv + t2*vertex2.uv) / invDepthW;

                    const int texelX = static_cast<int>(std::floor(uv.x * (spTexture.getWidth() - 1)));
                    const int texelY = static_cast<int>(std::floor(uv.y * (spTexture.getHeight() - 1)));

                    if (texelX >= 0 && texelX < spTexture.getWidth() &&
                        texelY >= 0 && texelY < spTexture.getHeight()
                    )
                    {
                        sp(x, invY) = spTexture(texelX, texelY);
                    }
                    else
                    {
                        sp(x, invY).data = 0xffff00ff;
                    }

                    spDepth(x, invY) = depth;
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

ADT_NO_UB static void
drawTriangle(
    const math::V4 p0, const math::V4 p1, const math::V4 p2,
    const math::V2 uv0, const math::V2 uv1, const math::V2 uv2,
    const Span2D<Pixel> spTexture
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

        drawTriangle2(v0, v1, v2, spTexture);
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
            Pixel pix {
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

[[maybe_unused]] static void
helloCubeTest()
{
    using namespace adt::math;

    auto& win = *app::g_pWindow;

    /* clear */
    win.clearBuffer();
    win.clearDepthBuffer();

    V3 aCubeVerts[] {
        /* Front Face */
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},

        /* Back Face */
        {-0.5f, -0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
    };

    V2 aCubeUVs[] {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1},

        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1},
    };

    int aIndices[][3] {
        /* Front Face */
        {0, 1, 2},
        {2, 3, 0},

        /* Back Face */
        {6, 5, 4},
        {4, 7, 6},

        /* Left face */
        {4, 5, 1},
        {1, 0, 4},

        /* Right face */
        {3, 2, 6},
        {6, 7, 3},

        /* Top face */
        {1, 5, 6},
        {6, 2, 1},

        /* Bottom face */
        {4, 0, 3},
        {3, 7, 4},
    };

    auto& camera = control::g_camera;
    f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);

    const f32 step = frame::g_time*0.0010;

    M4 tr = M4Pers(toRad(60.0f), aspectRatio, 0.01f, 1000.0f) *
        camera.m_trm *
        M4TranslationFrom(0.0f, 0.0f, -1.0f) *
        M4RotFrom(0, 0, step) *
        M4ScaleFrom(1.0f);

    static const Span2D<Pixel> spTexture = allocCheckerBoardTexture();

    Span spTransformedVertices = s_scratch.nextMem<V4>(utils::size(aCubeVerts));

    for (int vertexIdx = 0; vertexIdx < spTransformedVertices.getSize(); ++vertexIdx)
        spTransformedVertices[vertexIdx] = tr * V4From(aCubeVerts[vertexIdx], 1.0f);


    for (const auto [f0, f1, f2] : aIndices)
    {
        drawTriangle(
            spTransformedVertices[f0], spTransformedVertices[f1], spTransformedVertices[f2],
            aCubeUVs[f0], aCubeUVs[f1], aCubeUVs[f2],
            spTexture
        );
    }
}

void
toBuffer()
{
    f64 t0 = utils::timeNowMS();

    helloCubeTest();

    f64 t1 = utils::timeNowMS();
    CERR("helloCubeTest(): in {:.5} ms\n", t1 - t0);
}

} /* namespace draw */
