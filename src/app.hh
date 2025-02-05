#pragma once

#include "IWindow.hh"

#include "adt/utils.hh"
#include "adt/Thread.hh"

namespace app
{

enum class WINDOW_TYPE : adt::u8 { WAYLAND };

extern WINDOW_TYPE g_eWindowType;
extern IWindow* g_pWindow;

const static int NPROCS = adt::utils::min(4, ADT_GET_NCORES() / 2);

inline IWindow& window() { return *g_pWindow; }

/* create window based on `g_eWindowType` */
IWindow* allocWindow(adt::IAllocator* pAlloc, const char* ntsName);

} /* namespace app */;
