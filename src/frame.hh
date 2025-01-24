#pragma once

#include "adt/types.hh"

namespace frame
{

constexpr adt::u32 TICK_RATE = 240;
constexpr adt::f64 FIXED_DELTA_TIME = 1.0 / static_cast<adt::f64>(TICK_RATE);

extern adt::f64 g_currTime;
extern adt::f64 g_dt; /* delta time (fixed) */

void start();

} /* namespace frame */
