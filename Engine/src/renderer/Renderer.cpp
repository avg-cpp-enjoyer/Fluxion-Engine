#include <fluxion/renderer/Renderer.hpp>
#include <fluxion/core/GraphicsDevice.hpp>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma warning(disable: 26495)

namespace fluxion {
Renderer::Renderer(uint32 width, uint32 height) : m_width(width), m_height(height) {}

Renderer::~Renderer() {
	Shutdown();
}

bool Renderer::Initialize(platform::win32::WindowHandle hwnd) {
	FLUXION_ENSURE_RET(!m_initialized, false);
	FLUXION_ENSURE_RET(hwnd, false);
	FLUXION_ENSURE_RET(m_width > 0 && m_height > 0, false);

	FLUXION_ENSURE_RET(GraphicsDevice::Device(), false);
	m_device = GraphicsDevice::Device();

	FLUXION_ENSURE_RET(CreateSwapChain(hwnd), false);
	FLUXION_ENSURE_RET(CreateRTVHeap(), false);
	FLUXION_ENSURE_RET(CreateRenderTargets(), false);
	FLUXION_ENSURE_RET(CreateCommandObjects(), false);
	FLUXION_ENSURE_RET(CreateFence(), false);

	m_initialized = true;
	return true;
}

bool Renderer::CreateSwapChain(platform::win32::WindowHandle hwnd) {
	IDXGIFactory6* factory = GraphicsDevice::DXGIFactory();
	FLUXION_ENSURE_RET(factory, false);

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Width = m_width;
	desc.Height = m_height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = kFrameCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	FLUXION_CHECK(factory->CreateSwapChainForHwnd(GraphicsDevice::CommandQueue(), hwnd, &desc, nullptr, nullptr, &swapChain1));
	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	FLUXION_CHECK(swapChain1.As(&m_swapChain));
	return true;
}

bool Renderer::CreateRTVHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = kFrameCount;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	FLUXION_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap)));
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	return true;
}

bool Renderer::CreateRenderTargets() {
	for (uint32 i = 0; i < kFrameCount; ++i) {
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		FLUXION_CHECK(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)));

		auto& rt = m_renderTargets[i];
		if (!rt.Initialize(m_device, m_rtvHeap.Get(), i, m_rtvDescriptorSize, buffer.Get())) {
			return false;
		}
	}

	return true;
}

bool Renderer::CreateCommandObjects() {
	for (uint32 i = 0; i < kFrameCount; i++) {
		FLUXION_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
	}

	auto* alloc = m_commandAllocators[0].Get();
	FLUXION_CHECK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc, nullptr, IID_PPV_ARGS(&m_commandList)));
	FLUXION_CHECK(m_commandList->Close());
	return true;
}

bool Renderer::CreateFence() {
	FLUXION_CHECK(GraphicsDevice::Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	FLUXION_ENSURE_RET(m_fenceEvent, false);
	return true;
}

void Renderer::WaitForGPU() {
	if (!m_fence || !m_fenceEvent) {
		return;
	}

	const uint64 fenceValue = ++m_currentFenceValue;
	GraphicsDevice::CommandQueue()->Signal(m_fence.Get(), fenceValue);

	if (m_fence->GetCompletedValue() < fenceValue) {
		m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void Renderer::RenderFrame() {
	FLUXION_ENSURE(m_initialized);
	const uint32 frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValues[frameIndex]) {
		m_fence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_commandAllocators[frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[frameIndex].Get(), nullptr);

	RenderTarget& rt = m_renderTargets[frameIndex];
	rt.Transition(m_commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	rt.Clear(m_commandList.Get(), m_clearColor);
	rt.Transition(m_commandList.Get(), D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->Close();

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	GraphicsDevice::CommandQueue()->ExecuteCommandLists(1, lists);
	m_swapChain->Present(1, 0);

	const uint64 fenceValue = m_currentFenceValue++;
	GraphicsDevice::CommandQueue()->Signal(m_fence.Get(), fenceValue);
	m_fenceValues[frameIndex] = fenceValue;
}

void Renderer::Resize(uint32 width, uint32 height) {
	FLUXION_ENSURE(m_initialized);
	FLUXION_ENSURE(width > 0 && height > 0);
	if (width == m_width && height == m_height) {
		return;
	}

	WaitForGPU();

	for (auto& rt : m_renderTargets) {
		rt.Shutdown();
	}

	DXGI_SWAP_CHAIN_DESC1 desc{};
	m_swapChain->GetDesc1(&desc);
	long result = m_swapChain->ResizeBuffers(kFrameCount, width, height, desc.Format, desc.Flags);
	FLUXION_ENSURE(SUCCEEDED(result));

	m_width = width;
	m_height = height;

	FLUXION_ENSURE(CreateRenderTargets()); 

	for (auto& fv : m_fenceValues) {
		fv = m_currentFenceValue;
	}
}

void Renderer::Shutdown() {
	if (!m_initialized) {
		return;
	}

	WaitForGPU();

	for (auto& rt : m_renderTargets) {
		rt.Shutdown();
	}

	m_rtvHeap.Reset();

	if (m_fenceEvent) {
		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	m_fence.Reset();
	m_commandList.Reset();
	
	for (auto& ca : m_commandAllocators) {
		ca.Reset();
	}

	m_device = nullptr;
	m_swapChain.Reset();
	m_initialized = false;
}
}