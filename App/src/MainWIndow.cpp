#include "MainWIndow.hpp"

#include <Engine/include/fluxion/core/GraphicsDevice.hpp>

#pragma warning(disable: 6387)

namespace fluxion::app {
void MainWindow::Render() {
	FLUXION_SAFE_INVOKE(m_renderer, RenderFrame());
}

void MainWindow::Update(float32 dt) {
	if (m_renderer && m_mouseCaptured) {
		Camera& camera = m_renderer->GetCamera();
		DirectX::XMFLOAT3 res = InputResult();
		camera.MoveLocal(res.x, res.y, res.z, 5.0f * dt);
		camera.AddYawPitch(m_mouseDX * 0.002f, m_mouseDY * 0.002f);
	}

	m_mouseDX = 0.0f;
	m_mouseDY = 0.0f;
}

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
	case WM_KEYUP: 
		OnKeyUp(static_cast<uint32>(wParam));
		return 0;
	case WM_LBUTTONDOWN:
		CaptureMouse();
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove();
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
		FLUXION_MESSAGEBOX_ERROR(m_window, L"Renderer initialization failed.");
		PostMessage(m_window, WM_CLOSE, 0, 0);
	}
}

void MainWindow::OnDestroy() {
	if (m_mouseCaptured) {
		ShowCursor(TRUE);
	}

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
		OnVkEscape();
		break;
	default:
		m_keys[keyCode] = true;
		break;
	}
}

void MainWindow::OnKeyUp(uint32 keyCode) {
	m_keys[keyCode] = false;
}

void MainWindow::CaptureMouse() {
	if (m_mouseCaptured) {
		return;
	}

	m_mouseCaptured = true;
	ShowCursor(FALSE);
	GetCursorPos(&m_lastMousePos);
}

void MainWindow::OnMouseMove() {
	if (!m_mouseCaptured) {
		return;
	}

	RECT rc;
	GetClientRect(m_window, &rc);
	POINT center = { (rc.right - rc.left) >> 1, (rc.bottom - rc.top) >> 1 };
	ClientToScreen(m_window, &center);

	platform::win32::Point currentPos;
	GetCursorPos(&currentPos);
	FLUXION_ENSURE(currentPos.x != center.x || currentPos.y != center.y);

	m_mouseDX = static_cast<float32>(currentPos.x - center.x);
	m_mouseDY = static_cast<float32>(currentPos.y - center.y);
	SetCursorPos(center.x, center.y);
}

void MainWindow::OnVkEscape() {
	if (m_mouseCaptured) {
		m_mouseCaptured = false;
		ShowCursor(TRUE);
	} else {
		PostMessage(m_window, WM_CLOSE, 0, 0);
	}
}

DirectX::XMFLOAT3 MainWindow::InputResult() {
	float forward = 0.0f, right = 0.0f, up = 0.0f;

	if (m_keys['W']) forward += 1.0f;
	if (m_keys['S']) forward -= 1.0f;
	if (m_keys['D']) right += 1.0f;
	if (m_keys['A']) right -= 1.0f;
	if (m_keys[VK_SPACE]) up += 1.0f;
	if (m_keys[VK_CONTROL]) up -= 1.0f;

	return DirectX::XMFLOAT3(forward, right, up);
}
} 