#pragma once

#include <fluxion/common/Common.hpp>

#include "Win32Defs.hpp"

namespace fluxion::platform::win32 {
template <typename T>
class FLUXION_NOVTABLE Win32Base {
public:
	Win32Base() : m_window(nullptr) {}
	virtual ~Win32Base() noexcept = default;

	Win32Base(const Win32Base&) = delete;
	Win32Base& operator=(const Win32Base&) = delete;

	bool Create(
		const wchar* windowName, uint32 style, uint32 exStyle = 0,
		int32 x = CW_USEDEFAULT, int32 y = CW_USEDEFAULT, int32 width = CW_USEDEFAULT,
		int32 height = CW_USEDEFAULT, BrushHandle bgBrush = nullptr, WindowHandle parentWnd = nullptr,
		MenuHandle menu = nullptr
	);

	static iptr FLUXION_STDCALL WndProc(WindowHandle window, uint32 msg, uptr wParam, iptr lParam);
	WindowHandle Window() const noexcept;
protected:
	virtual const wchar* ClassName() const = 0;
	virtual iptr HandleMessage(uint32 msg, uptr wParam, iptr lParam) = 0;
protected:
	WindowHandle m_window;
};
}

#include "Win32Base.inl"