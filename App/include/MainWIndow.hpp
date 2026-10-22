#pragma once

#include <Engine/include/fluxion/renderer/Renderer.hpp>
#include <Engine/include/fluxion/platform/win32/Win32Base.hpp>
#include <Engine/include/fluxion/common/Common.hpp>

#include <optional>
#include <array>

namespace fluxion::app {
class MainWindow : public platform::win32::Win32Base<MainWindow> {
	friend class platform::win32::Win32Base<MainWindow>;
public:
	MainWindow() = default;
	~MainWindow() override = default;

	void Render();
	void Update(float32 dt);
protected:
	const wchar* ClassName() const override;
	iptr HandleMessage(uint32 msg, uptr wParam, iptr lParam) override;
private:
	void OnCreate();
	void OnDestroy();
	void OnSizeChanged(uint32 width, uint32 height);
	void OnKeyDown(uint32 keyCode);
	void OnKeyUp(uint32 keyCode);
	void CaptureMouse();
	void OnMouseMove();
	void OnVkEscape();
	DirectX::XMFLOAT3 InputResult();
private:
	std::optional<Renderer> m_renderer;
	std::array<bool, 256> m_keys = {};
	float m_mouseDX = 0.0f;
	float m_mouseDY = 0.0f;
	platform::win32::Point m_lastMousePos = { 0, 0 };
	bool m_mouseCaptured = false;
};
}