#pragma once

#include "adt/types.hh"

namespace frame
{

constexpr adt::i32 TICK_RATE = 120;
constexpr adt::f64 FIXED_DELTA_TIME = 1.0 / static_cast<adt::f64>(TICK_RATE);

extern adt::f64 g_time;
extern adt::f64 g_frameTime;
extern const adt::f64 g_dt; /* delta time (fixed) */
extern adt::f64 g_gameTime; /* game time */

void start();

} /* namespace frame */
