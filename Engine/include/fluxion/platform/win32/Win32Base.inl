namespace fluxion::platform::win32 {
template <typename T>
inline bool Win32Base<T>::Create(const wchar* windowName, uint32 style, uint32 exStyle,
	int32 x, int32 y, int32 width, int32 height, BrushHandle bgBrush,
	WindowHandle parentWnd, MenuHandle menu) {
	WindowClassEx wclassex{};

	wclassex.cbSize = sizeof(WindowClassEx);
	wclassex.style = CS_HREDRAW | CS_VREDRAW;
	wclassex.lpfnWndProc = Win32Base<T>::WndProc;
	wclassex.hInstance = GetModuleHandle(nullptr);
	wclassex.lpszClassName = ClassName();
	wclassex.hbrBackground = bgBrush;
	wclassex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wclassex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

	if (!RegisterClassEx(&wclassex) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
		return false;
	}

	m_window = CreateWindowEx(exStyle, ClassName(), windowName, style, x, y,
		width, height, parentWnd, menu, GetModuleHandle(nullptr), this
	);
	return m_window != nullptr;
}

template <typename T>
inline iptr Win32Base<T>::WndProc(WindowHandle window, uint32 msg, uptr wParam, iptr lParam) {
	T* self = nullptr;

	if (msg == WM_NCCREATE) {
		auto* create = reinterpret_cast<CreateStruct*>(lParam);
		self = static_cast<T*>(create->lpCreateParams);
		SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<iptr>(self));
		self->m_window = window;
	} else {
		self = reinterpret_cast<T*>(GetWindowLongPtr(window, GWLP_USERDATA));
	}

	if (self) {
		return self->HandleMessage(msg, wParam, lParam);
	}

	return DefWindowProc(window, msg, wParam, lParam);
}

template <typename T>
inline WindowHandle Win32Base<T>::Window() const noexcept {
	return m_window;
}
}