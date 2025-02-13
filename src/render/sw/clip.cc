#include "clip.hh"

using namespace adt;

namespace adt::print
{

[[maybe_unused]] static ssize
formatToContext(Context ctx, FormatArgs, const render::sw::clip::AXIS e)
{
    ctx.fmt = "{}";
    ctx.fmtIdx = 0;
    
    const String asMap[] {
        "NONE", "LEFT", "RIGHT", "TOP", "BOTTOM", "NEAR", "FAR", "W"
    };

    return printArgs(ctx, asMap[e]);
}

} /* namespace adt::print */

namespace render::sw::clip
{

constexpr f32 VERY_SMALL_NUMBER = 0.0001f;

static bool
behindPlane(math::V4 v, AXIS eAxis)
{
    bool bRes = false;

    switch (eAxis)
    {
        case AXIS::LEFT:
        bRes = v.x < -v.w;
        break;

        case AXIS::RIGHT:
        bRes = v.x > v.w;
        break;

        case AXIS::TOP:
        bRes = v.y > v.w;
        break;

        case AXIS::BOTTOM:
        bRes = v.y < -v.w;
        break;

        case AXIS::NEAR:
        bRes = v.z < 0;
        break;

        case AXIS::FAR:
        bRes = v.z > v.w;
        break;

        case AXIS::W:
        bRes = v.w < VERY_SMALL_NUMBER;
        break;

        default:
        ADT_ASSERT(false, "invalid code path");
    }

    return bRes;
}

static Vertex
calcIntersection(Vertex start, Vertex end, AXIS eAxis)
{
    Vertex res {};
    f32 s = 0.0f;

    const auto& startp = start.pos;
    const auto& endp = end.pos;

    switch (eAxis)
    {
        case AXIS::LEFT:
        s = -(startp.w + startp.x) / ((endp.x - startp.x) + (endp.w - startp.w));
        break;

        case AXIS::RIGHT:
        s = (startp.w - startp.x) / ((endp.x - startp.x) - (endp.w - startp.w));
        break;

        case AXIS::TOP:
        s = (startp.w - startp.y) / ((endp.y - startp.y) - (endp.w - startp.w));
        break;

        case AXIS::BOTTOM:
        s = -(startp.w + startp.y) / ((endp.y - startp.y) + (endp.w - startp.w));
        break;

        case AXIS::NEAR:
        s = -startp.z / (endp.z - startp.z);
        break;

        case AXIS::FAR:
        s = (startp.w - startp.z) / ((endp.z - startp.z) - (endp.w - startp.w));
        break;

        case AXIS::W:
        s = (VERY_SMALL_NUMBER - startp.w) / (endp.w - startp.w);
        break;

        default:
        ADT_ASSERT(false, "invalid code path");
    }

    res.pos = (1.0f - s) * start.pos + s * end.pos;
    res.uv = (1.0f - s) * start.uv + s * end.uv;

    return res;
}

void
polygonToAxis(const Result* pInput, Result* pOutput, const AXIS eAxis)
{
    pOutput->nTriangles = 0;

    for (int triangleIdx = 0; triangleIdx < pInput->nTriangles; ++triangleIdx)
    {
        const int aVertexIdx[3] {
            3 * triangleIdx + 0,
            3 * triangleIdx + 1,
            3 * triangleIdx + 2,
        };

        const Vertex aVertices[3] {
            pInput->aVertices[ aVertexIdx[0] ],
            pInput->aVertices[ aVertexIdx[1] ],
            pInput->aVertices[ aVertexIdx[2] ],
        };

        const bool abBehindPlane[3] {
            behindPlane(aVertices[0].pos, eAxis),
            behindPlane(aVertices[1].pos, eAxis),
            behindPlane(aVertices[2].pos, eAxis),
        };

        ssize nBehindPlane = abBehindPlane[0] + abBehindPlane[1] + abBehindPlane[2];

        switch (nBehindPlane)
        {
            case 0:
            {
                /* copy fully visible triangle */
                int currTriangleIdx = pOutput->nTriangles++;
                pOutput->aVertices[3*currTriangleIdx + 0] = aVertices[0];
                pOutput->aVertices[3*currTriangleIdx + 1] = aVertices[1];
                pOutput->aVertices[3*currTriangleIdx + 2] = aVertices[2];
            }
            break;

            case 1:
            {
                /* cut in two pieces */
                int currTriangleI = pOutput->nTriangles;
                int currVertexI = 0;
                bool bTriangleAdded = false;
                pOutput->nTriangles += 2;

                for (int edgeI = 0; edgeI < 3; ++edgeI)
                {
                    int startVertexI = edgeI;
                    int endVertexI = (edgeI == 2) ? 0 : (edgeI + 1);

                    Vertex startVertex = aVertices[startVertexI];
                    Vertex endVertex = aVertices[endVertexI];

                    bool bStartBehindPlane = abBehindPlane[startVertexI];
                    bool bEndBehindPlane = abBehindPlane[endVertexI];

                    if (!bStartBehindPlane)
                        pOutput->aVertices[3*currTriangleI + currVertexI++] = startVertex;

                    if (!bTriangleAdded && currVertexI == 3)
                    {
                        bTriangleAdded = true;
                        ++currTriangleI;
                        currVertexI = 0;
                        pOutput->aVertices[3*currTriangleI + currVertexI++] = pOutput->aVertices[3*(currTriangleI - 1) + 0];
                        pOutput->aVertices[3*currTriangleI + currVertexI++] = pOutput->aVertices[3*(currTriangleI - 1) + 2];
                    }

                    if (bStartBehindPlane != bEndBehindPlane)
                        pOutput->aVertices[3*currTriangleI + currVertexI++] = calcIntersection(startVertex, endVertex, eAxis);

                    if (!bTriangleAdded && currVertexI == 3)
                    {
                        bTriangleAdded = true;
                        ++currTriangleI;
                        currVertexI = 0;
                        pOutput->aVertices[3*currTriangleI + currVertexI++] = pOutput->aVertices[3*(currTriangleI - 1) + 0];
                        pOutput->aVertices[3*currTriangleI + currVertexI++] = pOutput->aVertices[3*(currTriangleI - 1) + 2];
                    }
                }
            }
            break;

            case 2:
            {
                int currTriangleIdx = pOutput->nTriangles++;
                int currVertexIdx = 0;

                for (int edgeIdx = 0; edgeIdx < 3; ++edgeIdx)
                {
                    int startVertexIdx = edgeIdx;
                    int endVertexIdx = edgeIdx == 2 ? 0 : edgeIdx + 1;

                    Vertex startVertex = aVertices[startVertexIdx];
                    Vertex endVertex = aVertices[endVertexIdx];

                    bool bStartBehindPlane = abBehindPlane[startVertexIdx];
                    bool bEndBehindPlane = abBehindPlane[endVertexIdx];

                    if (!bStartBehindPlane)
                    {
                        pOutput->aVertices[3*currTriangleIdx + currVertexIdx++] = startVertex;
                    }

                    if (bStartBehindPlane != bEndBehindPlane)
                    {
                        pOutput->aVertices[3*currTriangleIdx + currVertexIdx++] = calcIntersection(startVertex, endVertex, eAxis);
                    }
                }
            }
            break;

            case 3:
            {
                /* don't add */
            }
            break;

            default:
            ADT_ASSERT(false, "invalid code path");
        }
    }
}

} /* namespace render::sw::clip */
