#pragma once

#include "IWindow.hh"
#include "render/IRenderer.hh"

#include "adt/ThreadPool.hh"
#include "adt/ScratchBuffer.hh"

namespace app
{

enum class WINDOW_TYPE : adt::u8 { WAYLAND_SHM, WAYLAND_GL, WINDOWS };
enum class RENDERER_TYPE : adt::u8 { SW, OPEN_GL };

/* depend on global `g_eWindowType` and `g_eRendererType` */
IWindow* allocWindow(adt::IAllocator* pAlloc, const char* ntsName);
render::IRenderer* allocRenderer(adt::IAllocator* pAlloc);

extern WINDOW_TYPE g_eWindowType;
extern RENDERER_TYPE g_eRendererType;

extern IWindow* g_pWindow;
inline IWindow& windowInst() { return *g_pWindow; }

extern render::IRenderer* g_pRenderer;
inline render::IRenderer& rendererInst() { return *g_pRenderer; }

void allocScratchForThisThread(adt::ssize size);
void destroyScratchForThisThread();

extern thread_local adt::ScratchBuffer gtl_scratch;

extern adt::ThreadPool g_threadPool;

} /* namespace app */;
