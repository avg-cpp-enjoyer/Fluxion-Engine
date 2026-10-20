#include "App.hpp"

#include <Engine/include/fluxion/core/GraphicsDevice.hpp>
#include <Engine/include/fluxion/core/DebugLayer.hpp> 

namespace fluxion::app {
App::~App() {
	fluxion::GraphicsDevice::Shutdown();
#if defined(_DEBUG)
	fluxion::DebugLayer::DumpLiveObjects();
	fluxion::DebugLayer::Shutdown();
#endif
}

bool App::Initialize(uint32 width, uint32 height) {
#if defined(_DEBUG)
	fluxion::DebugLayer::Enable();
#endif
	if (!fluxion::GraphicsDevice::Initialize()) {
		MessageBoxW(nullptr, L"GraphicsDevice init failed. See debug output.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (!m_mainWindow.Create(L"Fluxion Engine", WS_OVERLAPPEDWINDOW, 0, CW_USEDEFAULT, CW_USEDEFAULT, width, height)) {
		return false;
	}

	return true;
}

int App::Exec(int cmdShow) {
	ShowWindow(m_mainWindow.Window(), cmdShow);
	UpdateWindow(m_mainWindow.Window());

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		} else {
			m_mainWindow.Render();
		}
	}

	if (m_mainWindow.Window()) {
		DestroyWindow(m_mainWindow.Window());
	}

	return static_cast<int>(msg.wParam);
}
}