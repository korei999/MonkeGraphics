#pragma once

#include "IWindow.hh"

namespace app
{

enum class WINDOW_TYPE : adt::u8 { WAYLAND, WINDOWS };

extern WINDOW_TYPE g_eWindowType;
extern IWindow* g_pWindow;

inline IWindow& window() { return *g_pWindow; }

/* create window based on `g_eWindowType` */
IWindow* allocWindow(adt::IAllocator* pAlloc, const char* ntsName);

} /* namespace app */;
