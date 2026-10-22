#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

namespace fluxion::platform::win32 {
using WindowHandle = HWND;
using InstanceHandle = HINSTANCE;
using ModuleHandle = HMODULE;
using BrushHandle = HBRUSH;
using MenuHandle = HMENU;
using WindowClassEx = WNDCLASSEX;
using CreateStruct = CREATESTRUCT;
using Point = POINT;
}