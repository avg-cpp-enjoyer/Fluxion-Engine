#include "App.hpp"

#include <Engine/include/fluxion/core/GraphicsDevice.hpp>
#include <Engine/include/fluxion/core/DebugLayer.hpp> 

#include <chrono>

namespace fluxion::app {
App::~App() {
	fluxion::GraphicsDevice::Shutdown();
#if defined(_DEBUG)
	fluxion::DebugLayer::DumpLiveObjects();
	fluxion::DebugLayer::Shutdown();
#endif
}

bool App::Initialize(uint32 width, uint32 height) {
	FLUXION_CHECK(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
#if defined(_DEBUG)
	fluxion::DebugLayer::Enable();
#endif
	if (!fluxion::GraphicsDevice::Initialize()) {
		FLUXION_MESSAGEBOX_ERROR(nullptr, L"GraphicsDevice init failed. See debug output.");
		return false;
	}

	FLUXION_ENSURE_RET(m_mainWindow.Create(L"Fluxion Engine", WS_OVERLAPPEDWINDOW, 0, CW_USEDEFAULT, CW_USEDEFAULT, width, height), false);
	return true;
}

int App::Exec(int cmdShow) {
	auto lastTime = std::chrono::high_resolution_clock::now();

	ShowWindow(m_mainWindow.Window(), cmdShow);
	UpdateWindow(m_mainWindow.Window());

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		} else {
			auto currentTime = std::chrono::high_resolution_clock::now();
			float32 dt = std::chrono::duration<float32>(currentTime - lastTime).count();
			lastTime = currentTime;

			//TODO: Would like to move rendering to a dedicated thread but it's gonna be literal hell implementing it on DX12 so i'll leave it here for now 
			m_mainWindow.Update(dt);
			m_mainWindow.Render();
		}
	}

	if (m_mainWindow.Window()) {
		DestroyWindow(m_mainWindow.Window());
	}

	return static_cast<int32>(msg.wParam);
}
}