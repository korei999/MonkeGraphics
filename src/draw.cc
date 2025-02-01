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

    const int minX = utils::min(
        utils::min(static_cast<int>(pointA.x), static_cast<int>(pointB.x)),
        static_cast<int>(pointC.x)
    );
    const int maxX = utils::max(
        utils::max(static_cast<int>(std::round(pointA.x)), static_cast<int>(std::round(pointB.x))),
        static_cast<int>(std::round(pointC.x))
    );
    const int minY = utils::min(
        utils::min(static_cast<int>(pointA.y), static_cast<int>(pointB.y)),
        static_cast<int>(pointC.y)
    );
    const int maxY = utils::max(
        utils::max(static_cast<int>(std::round(pointA.y)), static_cast<int>(std::round(pointB.y))),
        static_cast<int>(std::round(pointC.y))
    );

    // minX = utils::clamp(minX, 0, static_cast<int>(sp.getWidth() - 1));
    // maxX = utils::clamp(maxX, 0, static_cast<int>(sp.getWidth() - 1));
    // minY = utils::clamp(minY, 0, static_cast<int>(sp.getHeight() - 1));
    // maxY = utils::clamp(maxY, 0, static_cast<int>(sp.getHeight() - 1));

    const bool bTopLeft0 = (edge0.y > 0.0f) || (edge0.x > 0.0f && edge0.y == 0.0f);
    const bool bTopLeft1 = (edge1.y > 0.0f) || (edge1.x > 0.0f && edge1.y == 0.0f);
    const bool bTopLeft2 = (edge2.y > 0.0f) || (edge2.x > 0.0f && edge2.y == 0.0f);

    const f32 barycentricDiv = V2Cross(pointB - pointA, pointC - pointA);

    vertex0.uv *= vertex0.pos.w;
    vertex1.uv *= vertex1.pos.w;
    vertex2.uv *= vertex2.pos.w;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            const V2 pixPoint = V2{
                static_cast<f32>(x),
                static_cast<f32>(y)
            } + V2{0.5f, 0.5f};

            const V2 pixEdge0 = pixPoint - pointA; 
            const V2 pixEdge1 = pixPoint - pointB; 
            const V2 pixEdge2 = pixPoint - pointC; 

            const f32 crossLen0 = V2Cross(pixEdge0, edge0);
            const f32 crossLen1 = V2Cross(pixEdge1, edge1);
            const f32 crossLen2 = V2Cross(pixEdge2, edge2);

            /* inside triangle */
            if ((crossLen0 > 0.0f || (bTopLeft0 && crossLen0 == 0.0f)) &&
                (crossLen1 > 0.0f || (bTopLeft1 && crossLen1 == 0.0f)) &&
                (crossLen2 > 0.0f || (bTopLeft2 && crossLen2 == 0.0f))
            )
            {
                const ssize invY = sp.getHeight() - 1 - y;

                const f32 t0 = -crossLen1 / barycentricDiv;
                const f32 t1 = -crossLen2 / barycentricDiv;
                const f32 t2 = -crossLen0 / barycentricDiv;

                f32 depth = t0*vertex0.pos.z + t1*vertex1.pos.z + t2*vertex2.pos.z;
                if (depth >= 0.0f && depth <= 1.0f && depth < spDepth(x, invY))
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
        }
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

        drawTriangle(v0, v1, v2, spTexture);
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

    f32 step = frame::g_time*0.0010;

    M4 tr = M4Pers(toRad(60.0f), aspectRatio, 0.01f, 1000.0f) *
        camera.m_trm *
        M4TranslationFrom(0.0f, 0.0f, -1.0f) *
        M4RotFrom(step, step, step) *
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
    f32 t0 = utils::timeNowMS();

    helloCubeTest();

    f32 t1 = utils::timeNowMS();
    CERR("helloCubeTest(): in {:.5} ms\n", t1 - t0);
}

} /* namespace draw */
