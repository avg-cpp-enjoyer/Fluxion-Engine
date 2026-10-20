#include <fluxion/core/GraphicsDevice.hpp>
#include <fluxion/core/DebugLayer.hpp>
#include <fluxion/common/Common.hpp>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <cstdio>
#include <format>

namespace fluxion {
bool GraphicsDevice::Initialize() {
	if (m_initialized) {
		return true;
	}

	FLUXION_ENSURE_RET(InitDXGIFactory(), false);
	FLUXION_ENSURE_RET(InitD3D12Device(), false);
	FLUXION_ENSURE_RET(InitCommandQueue(), false);

	m_initialized = true;
	return true;
}

bool GraphicsDevice::InitDXGIFactory() {
	uint32 factoryFlags = 0;
#if defined(_DEBUG)
	factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	FLUXION_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));
	return true;
}

Microsoft::WRL::ComPtr<IDXGIAdapter4> GraphicsDevice::PickBestAdapter() {
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
	if (!SUCCEEDED(m_dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))) {
		return nullptr;
	}

	DXGI_ADAPTER_DESC3 desc{};
	adapter->GetDesc3(&desc);
	OutputDebugString(std::format(L"Chosen adapter: {} ({} GB VRAM)\n", desc.Description, desc.DedicatedVideoMemory >> 30).c_str());
	return adapter;
}

bool GraphicsDevice::InitD3D12Device() {
	FLUXION_CHECK(D3D12CreateDevice(PickBestAdapter().Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
	DebugLayer::InitializeForDevice(m_device.Get());
	return true;
}

bool GraphicsDevice::InitCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	FLUXION_CHECK(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue)));
	return true;
}

void GraphicsDevice::Shutdown() {
	if (!m_initialized) {
		return;
	}

	m_commandQueue.Reset();
	m_device.Reset();
	m_dxgiFactory.Reset();
	m_initialized = false;
}

IDXGIFactory6* GraphicsDevice::DXGIFactory() { 
	return m_dxgiFactory.Get(); 
}

ID3D12Device* GraphicsDevice::Device() { 
	return m_device.Get(); 
}

ID3D12CommandQueue* GraphicsDevice::CommandQueue() { 
	return m_commandQueue.Get(); 
}
} 