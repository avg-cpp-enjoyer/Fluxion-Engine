#pragma once

#include <wrl/client.h>
#include <d3d12.h>    
#include <dxgidebug.h> 
#include <dxgi1_6.h>

namespace fluxion {
class GraphicsDevice {
public:
	static bool Initialize();
	static void Shutdown();
	static IDXGIFactory6* DXGIFactory();
	static ID3D12Device* Device();
	static ID3D12CommandQueue* CommandQueue();
private:
	static bool InitDXGIFactory();
	static Microsoft::WRL::ComPtr<IDXGIAdapter4> PickBestAdapter();
	static bool InitD3D12Device();
	static bool InitCommandQueue();
private:
	static inline Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	static inline Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	static inline Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;
	static inline bool m_initialized = false;
};
} 