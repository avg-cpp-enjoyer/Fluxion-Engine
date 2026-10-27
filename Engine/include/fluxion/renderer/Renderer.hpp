#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/renderer/RenderTarget.hpp>
#include <fluxion/renderer/Camera.hpp>
#include <fluxion/renderer/ConstantBuffer.hpp>
#include <fluxion/renderer/ConstantBuffers.hpp>
#include <fluxion/core/RingBuffer.hpp>
#include <fluxion/core/SRVHeap.hpp>
#include <fluxion/renderer/RenderItem.hpp>
#include <fluxion/renderer/GBuffer.hpp>
#include <fluxion/renderer/graph/RenderGraph.hpp>
#include <fluxion/renderer/RenderContext.hpp>
#include <fluxion/renderer/passes/GeometryPass.hpp>
#include <fluxion/renderer/passes/LightingPass.hpp>
#include <fluxion/renderer/passes/GridPass.hpp>
#include <fluxion/platform/win32/Win32Defs.hpp>

#include <wrl/client.h>
#include <vector>
#include <memory>
#include <dxgi1_6.h>
#include <optional>

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
	Camera& GetCamera();
private:
	bool CreateSwapChain(platform::win32::WindowHandle hwnd);
	bool CreateRTVHeap();
	bool CreateRenderTargets();
	bool CreateCommandObjects();
	bool CreateFence();
	bool CreateDepthBuffer();
	bool CreateBuffers();
	bool LoadShaders();
	void WaitForGPU();
private:
	static constexpr uint32 kFrameCount = 2;
	uint32 m_width = 0;
	uint32 m_height = 0;
	ID3D12Device* m_device;
	ID3D12CommandQueue* m_queue;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[kFrameCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsv{};
	RenderTarget m_renderTargets[2];
	uint32 m_rtvDescriptorSize = 0;
	uint64 m_fenceValues[kFrameCount]{};
	uint64 m_currentFenceValue = 0;
	void* m_fenceEvent = nullptr;
	float m_clearColor[4] = { 0.05f, 0.10f, 0.20f, 1.0f };
	bool m_initialized = false;

	RingBuffer m_transformBuffer;
	RingBuffer m_materialBuffer;
	RingBuffer m_lightBuffer;
	ConstantBuffer<CameraParams> m_cameraBuffer;
	ConstantBuffer<GridParams> m_gridParamsBuffer;
	Camera m_camera;
	GBuffer m_gbuffer;
	std::vector<RenderItem> m_renderItems;
	RenderItem m_grid;

	RenderGraph m_renderGraph;
	RenderContext m_renderContext;
	std::optional<RGResource> m_swapChainRG;

	std::unique_ptr<GeometryPass> m_geometryPass;
	std::unique_ptr<LightingPass> m_lightingPass;
	std::unique_ptr<GridPass> m_gridPass;
	std::optional<RGResource> m_depthRG;
	SRVHeap m_srvHeap;
	uint32 m_depthSRVIndex = UINT32_MAX;
};
}