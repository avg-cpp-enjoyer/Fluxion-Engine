#pragma once

#include <fluxion/common/Common.hpp>

#include <d3d12.h>
#include <wrl/client.h>
#include <cstring>

namespace fluxion {
class RingBuffer {
public:
	bool Initialize(ID3D12Device* device, uint32 sizeInBytes);
	void Reset();
	D3D12_GPU_VIRTUAL_ADDRESS AllocateAndCopy(const void* data, uint32 sizeInBytes);
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
	uint8* m_mappedPtr = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress = 0;
	uint32 m_sizeInBytes = 0;
	uint32 m_currentOffset = 0;
};
}