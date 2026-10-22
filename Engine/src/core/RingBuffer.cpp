#include <fluxion/core/RingBuffer.hpp>
#include <directx/d3dx12.h>

namespace fluxion {
bool RingBuffer::Initialize(ID3D12Device* device, uint32 sizeInBytes) {
	m_sizeInBytes = sizeInBytes;
	m_currentOffset = 0;

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_sizeInBytes);

	FLUXION_CHECK(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer))
	);
	m_buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedPtr));
	m_gpuAddress = m_buffer->GetGPUVirtualAddress();
	return true;
}

void RingBuffer::Reset() {
	m_currentOffset = 0;
}

D3D12_GPU_VIRTUAL_ADDRESS RingBuffer::AllocateAndCopy(const void* data, uint32 sizeInBytes) {
	uint32 alignedSize = (sizeInBytes + 255) & ~255;
	if (m_currentOffset + alignedSize > m_sizeInBytes) {
		m_currentOffset = 0;
	}

	uint8* destCpu = m_mappedPtr + m_currentOffset;
	memcpy(destCpu, data, sizeInBytes);

	D3D12_GPU_VIRTUAL_ADDRESS gpuAddr = m_gpuAddress + m_currentOffset;
	m_currentOffset += alignedSize;
	return gpuAddr;
}
}
