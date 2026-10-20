#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/renderer/RenderTarget.hpp>
#include <fluxion/platform/win32/Win32Defs.hpp>
#include <wrl/client.h>

struct IDXGISwapChain3;
struct ID3D12DescriptorHeap;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;
struct ID3D12Device;

namespace fluxion {
class Renderer {
public:
	Renderer(uint32 width, uint32 height);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	bool Initialize(platform::win32::WindowHandle hwnd);
	void Resize(uint32 width, uint32 height);
	void RenderFrame();
	void Shutdown();
private:
	bool CreateSwapChain(platform::win32::WindowHandle hwnd);
	bool CreateRTVHeap();
	bool CreateRenderTargets();
	bool CreateCommandObjects();
	bool CreateFence();
	void WaitForGPU();
private:
	static constexpr uint32 kFrameCount = 2;
	uint32 m_width = 0;
	uint32 m_height = 0;
	ID3D12Device* m_device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[kFrameCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	RenderTarget m_renderTargets[2];
	uint32 m_rtvDescriptorSize = 0;
	uint64 m_fenceValues[kFrameCount]{};
	uint64 m_currentFenceValue = 0;
	HANDLE m_fenceEvent = nullptr;
	float m_clearColor[4] = { 0.05f, 0.10f, 0.20f, 1.0f };
	bool  m_initialized = false;
};
}