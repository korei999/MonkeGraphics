#pragma once

#include "adt/math.hh"

namespace render::sw::clip
{

constexpr int MAX_NUM_VERTICES = 128;

enum AXIS : adt::u8 {NONE, LEFT, RIGHT, TOP, BOTTOM, NEAR, FAR, W};

struct Vertex
{
    adt::math::V4 pos {};
    adt::math::V2 uv {};
};

struct Result
{
    int nTriangles {};
    Vertex aVertices[MAX_NUM_VERTICES] {};
};

void polygonToAxis(const Result* pInput, Result* pOutput, const AXIS eAxis);

} /* namespace render::sw::clip */
