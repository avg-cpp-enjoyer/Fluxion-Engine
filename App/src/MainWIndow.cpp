#include "MainWIndow.hpp"

#include <Engine/include/fluxion/core/GraphicsDevice.hpp>

#pragma warning(disable: 6387)

namespace fluxion::app {
const wchar* MainWindow::ClassName() const {
	return L"FluxionMainWindow";
}

iptr MainWindow::HandleMessage(uint32 msg, uptr wParam, iptr lParam) {
	switch (msg) {
	case WM_CREATE:
		OnCreate();
		return 0;
	case WM_DESTROY:
		OnDestroy();
		return 0;
	case WM_SIZE:
		OnSizeChanged(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_ERASEBKGND:
		return 1; 
	case WM_KEYDOWN:
		OnKeyDown(static_cast<uint32>(wParam));
		return 0;
	}

	return DefWindowProc(m_window, msg, wParam, lParam);
}

void MainWindow::OnCreate() {
	RECT rc{};
	GetClientRect(m_window, &rc);
	uint32 width = static_cast<uint32>(rc.right - rc.left);
	uint32 height = static_cast<uint32>(rc.bottom - rc.top);

	m_renderer.emplace(width, height);
	if (!m_renderer->Initialize(m_window)) {
		MessageBoxW(m_window, L"Renderer initialization failed.", L"Fluxion Error", MB_OK | MB_ICONERROR);
		PostMessage(m_window, WM_CLOSE, 0, 0);
	}
}

void MainWindow::OnDestroy() {
	m_renderer.reset();
	PostQuitMessage(0);
}

void MainWindow::OnSizeChanged(uint32 width, uint32 height) {
	FLUXION_ENSURE(width > 0 && height > 0);
	FLUXION_SAFE_INVOKE(m_renderer, Resize(width, height));
}

void MainWindow::OnKeyDown(uint32 keyCode) {
	switch (keyCode) {
	case VK_ESCAPE:
		PostMessage(m_window, WM_CLOSE, 0, 0);
		break;
	default:
		break;
	}
}

void MainWindow::Render() {
	FLUXION_SAFE_INVOKE(m_renderer, RenderFrame());
}
} 