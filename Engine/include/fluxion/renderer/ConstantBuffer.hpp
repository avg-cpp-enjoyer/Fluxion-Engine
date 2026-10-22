#pragma once

#include <fluxion/common/Common.hpp>
#include <directx/d3dx12.h> 

#include <d3d12.h>
#include <wrl/client.h>

namespace fluxion {
template <typename T> requires(sizeof(T) % 256 == 0)
class ConstantBuffer {
public:
	ConstantBuffer() = default;
	~ConstantBuffer() = default;

	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;

	bool Initialize(ID3D12Device* device);
	T& GetData();
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
	T* m_mappedData = nullptr;
};
}

#include "ConstantBuffer.inl"