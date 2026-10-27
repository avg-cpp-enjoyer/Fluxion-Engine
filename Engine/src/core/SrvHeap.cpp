#include <fluxion/core/SrvHeap.hpp>
#include <fluxion/core/GraphicsDevice.hpp>

namespace fluxion {
bool SRVHeap::Initialize(ID3D12Device* device) {
	FLUXION_ENSURE_RET(device, false);

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = kCapacity;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	FLUXION_CHECK(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
	m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_nextFreeSlot = 0;
	return true;
}

uint32 SRVHeap::AllocateSlot() {
	FLUXION_ENSURE_RET(m_nextFreeSlot < kCapacity, 0);
	return m_nextFreeSlot++;
}

void SRVHeap::WriteTexture2D(uint32 index, ID3D12Resource* resource, DXGI_FORMAT format, uint32 mipLevels) {
	FLUXION_ENSURE(index < kCapacity);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += index * m_descriptorSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = mipLevels;
	GraphicsDevice::Device()->CreateShaderResourceView(resource, &srvDesc, handle);
}

ID3D12DescriptorHeap* SRVHeap::GetHeap() const {
	return m_heap.Get();
}

uint32 SRVHeap::GetDescriptorSize() const {
	return m_descriptorSize;
}
}

