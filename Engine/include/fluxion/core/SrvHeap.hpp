#pragma once

#include <fluxion/common/Common.hpp>

#include <d3d12.h>
#include <wrl/client.h>

namespace fluxion {
class SRVHeap {
public:
	bool Initialize(ID3D12Device* device);
	uint32 AllocateSlot();
	void WriteTexture2D(uint32 index, ID3D12Resource* resource, DXGI_FORMAT format, uint32 mipLevels = 1);
	ID3D12DescriptorHeap* GetHeap() const;
	uint32 GetDescriptorSize() const;
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	uint32 m_descriptorSize = 0;
	uint32 m_nextFreeSlot = 0;
	static constexpr uint32 kCapacity = 4096;
};


} 