#pragma once

#include <Engine/include/fluxion/renderer/Renderer.hpp>
#include <Engine/include/fluxion/platform/win32/Win32Base.hpp>
#include <Engine/include/fluxion/common/Common.hpp>

#include <optional>

namespace fluxion::app {
class MainWindow : public platform::win32::Win32Base<MainWindow> {
	friend class platform::win32::Win32Base<MainWindow>;
public:
	MainWindow() = default;
	~MainWindow() override = default;

	void Render();
protected:
	const wchar* ClassName() const override;
	iptr HandleMessage(uint32 msg, uptr wParam, iptr lParam) override;
private:
	void OnCreate();
	void OnDestroy();
	void OnSizeChanged(uint32 width, uint32 height);
	void OnKeyDown(uint32 keyCode);
private:
	std::optional<Renderer> m_renderer;
};
}