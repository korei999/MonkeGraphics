/* Rasterization goes here */

#include "adt/OsAllocator.hh"
#include "adt/math.hh"

#include "app.hh"
#include "colors.hh"
#include "control.hh"
#include "draw.hh"
#include "frame.hh"

using namespace adt;

namespace draw
{

static Span2D<Pixel>
checkerBoardTexture()
{
    const auto& win = *app::g_pWindow;
    OsAllocator al;

    Pixel* pData = (Pixel*)al.malloc(win.m_width * win.m_height, sizeof(*pData));
    Span2D sp(pData, win.m_width, win.m_height);
    for (int y = 0; y < sp.getHeight(); ++y)
    {
        for (int x = 0; x < sp.getWidth(); ++x)
        {
            u32 colorChannel = 255 * ((x + (y % 2)) % 2);
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
    math::V3 pPoints[3],
    math::V3 pColors[3],
    const math::M4& trm,
    bool bVerticalFlip = true
)
{
    using namespace adt::math;

    auto& win = *app::g_pWindow;
    Span2D sp = win.surfaceBuffer();
    Span2D spDepth = win.depthBuffer();

    V4 trPoint0 = (trm * V4From(pPoints[0], 1.0f));
    V4 trPoint1 = (trm * V4From(pPoints[1], 1.0f));
    V4 trPoint2 = (trm * V4From(pPoints[2], 1.0f));

    trPoint0.xyz /= trPoint0.w;
    trPoint1.xyz /= trPoint1.w;
    trPoint2.xyz /= trPoint2.w;

    V2 pointA = ndcToPix(trPoint0.xy);
    V2 pointB = ndcToPix(trPoint1.xy);
    V2 pointC = ndcToPix(trPoint2.xy);

    V2 edge0 = pointB - pointA;
    V2 edge1 = pointC - pointB;
    V2 edge2 = pointA - pointC;

    if (V2Cross(edge0, pointC - pointA) > 0)
        return;

    int minX = utils::min(utils::min(static_cast<int>(pointA.x),             static_cast<int>(pointB.x)),             static_cast<int>(pointC.x));
    int maxX = utils::max(utils::max(static_cast<int>(std::round(pointA.x)), static_cast<int>(std::round(pointB.x))), static_cast<int>(std::round(pointC.x)));
    int minY = utils::min(utils::min(static_cast<int>(pointA.y),             static_cast<int>(pointB.y)),             static_cast<int>(pointC.y));
    int maxY = utils::max(utils::max(static_cast<int>(std::round(pointA.y)), static_cast<int>(std::round(pointB.y))), static_cast<int>(std::round(pointC.y)));

    minX = utils::clamp(minX, 0, static_cast<int>(sp.getWidth() - 1));
    maxX = utils::clamp(maxX, 0, static_cast<int>(sp.getWidth() - 1));
    minY = utils::clamp(minY, 0, static_cast<int>(sp.getHeight() - 1));
    maxY = utils::clamp(maxY, 0, static_cast<int>(sp.getHeight() - 1));

    bool bTopLeft0 = (edge0.y > 0.0f) || (edge0.x > 0.0f && edge0.y == 0.0f);
    bool bTopLeft1 = (edge1.y > 0.0f) || (edge1.x > 0.0f && edge1.y == 0.0f);
    bool bTopLeft2 = (edge2.y > 0.0f) || (edge2.x > 0.0f && edge2.y == 0.0f);

    f32 barycentricDiv = V2Cross(pointB - pointA, pointC - pointA);

    for (ssize y = minY; y <= maxY; ++y)
    {
        for (ssize x = minX; x <= maxX; ++x)
        {
            V2 pixPoint = V2{static_cast<f32>(x), static_cast<f32>(y)} + V2{0.5f, 0.5f};

            V2 pixEdge0 = pixPoint - pointA; 
            V2 pixEdge1 = pixPoint - pointB; 
            V2 pixEdge2 = pixPoint - pointC; 

            f32 crossLen0 = V2Cross(pixEdge0, edge0);
            f32 crossLen1 = V2Cross(pixEdge1, edge1);
            f32 crossLen2 = V2Cross(pixEdge2, edge2);

            /* inside triangle */
            if ((crossLen0 > 0.0f || (bTopLeft0 && crossLen0 == 0.0f)) &&
                (crossLen1 > 0.0f || (bTopLeft1 && crossLen1 == 0.0f)) &&
                (crossLen2 > 0.0f || (bTopLeft2 && crossLen2 == 0.0f))
            )
            {
                ssize invY = bVerticalFlip ? sp.getHeight() - 1 - y : y;

                f32 t0 = -crossLen1 / barycentricDiv;
                f32 t1 = -crossLen2 / barycentricDiv;
                f32 t2 = -crossLen0 / barycentricDiv;

                f32 depth = t0 * trPoint0.z + t1 * trPoint1.z + t2 * trPoint2.z;
                if (depth >= 0.0f && depth <= 1.0f && depth < spDepth(x, invY))
                {
                    // V2 uv = t0 * pUV[0] + t1 * pUV[1] + t2 * pUV[2];
                    // int texelX = std::floor(uv.x * tex.width - 1);
                    // int texelY = std::floor(uv.y * tex.height - 1);

                    V3 finalCol = (t0 * pColors[0] + t1 * pColors[1] + t2 * pColors[2]);
                    V4 cv4; cv4.xyz = finalCol; cv4.w = 255.0f;

                    sp(x, invY).data = colors::V4ToARGB(cv4);
                    spDepth(x, invY) = depth;
                }
            }
        }
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
    Span2D sp = win.surfaceBuffer();

    /* clear */
    win.clearBuffer();
    win.clearDepthBuffer();

    V3 aCubeVerts[] {
        /* front face */
        {-0.5f,  0.5f, -0.5f}, /* tl */
        { 0.5f, -0.5f, -0.5f}, /* br */
        {-0.5f, -0.5f, -0.5f}, /* bl */
        { 0.5f,  0.5f, -0.5f}, /* tr */
        /* */

        /* back face */
        {-0.5f,  0.5f, 0.5f}, /* tl */
        { 0.5f, -0.5f, 0.5f}, /* br */
        {-0.5f, -0.5f, 0.5f}, /* bl */
        { 0.5f,  0.5f, 0.5f}, /* tr */
        /* */
    };

    V3 aVertColors[] {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
        {1, 1, 0},
    };

    int aIndexBuff[][3] {
        {0, 1, 2},
        {0, 3, 1},

        {4, 6, 5},
        {5, 7, 4},

        {0, 2, 4},
        {2, 6, 4},

        {3, 7, 5},
        {3, 5, 1},

        {0, 4, 3},
        {3, 4, 7},

        {2, 1, 6},
        {1, 5, 6},
    };

    auto& camera = control::g_camera;
    f32 aspectRatio = static_cast<f32>(win.m_winWidth) / static_cast<f32>(win.m_winHeight);

    f32 step = frame::g_time*0.0015;

    M4 tr = M4Pers(toRad(60.0f), aspectRatio, 0.01f, 500.0f) *
        camera.m_trm *
        M4TranslationFrom(0.0f, 0.0f, 2.5f) *
        M4RotFrom(step, step, step) *
        M4ScaleFrom(1.0f);

    for (const auto& [f0, f1, f2] : aIndexBuff)
    {
        V3 aTriangle[] { aCubeVerts[f0], aCubeVerts[f1], aCubeVerts[f2] };
        V3 aColors[] { aVertColors[f0 % 4], aVertColors[f1 % 4], aVertColors[f2 % 4] };

        drawTriangle(aTriangle, aColors, tr);
    }
}

void
toBuffer()
{
    helloCubeTest();
}

} /* namespace draw */
