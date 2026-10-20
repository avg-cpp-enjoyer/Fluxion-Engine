#pragma once

#include <fluxion/common/Common.hpp>
#include <wrl/client.h>
#include <d3d12.h>

namespace fluxion {
class RenderTarget {
public:
	RenderTarget() = default;
	~RenderTarget() = default;

	RenderTarget(const RenderTarget&) = delete;
	RenderTarget& operator=(const RenderTarget&) = delete;
	RenderTarget(RenderTarget&&) noexcept = default;
	RenderTarget& operator=(RenderTarget&&) noexcept = default;

	bool Initialize(
		ID3D12Device* device,
		ID3D12DescriptorHeap* rtvHeap,
		uint32 rtvIndex,
		uint32 rtvDescriptorSize,
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_PRESENT
	);

	void Shutdown();
	void Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES newState);
	void Clear(ID3D12GraphicsCommandList* cmdList, const float color[4]);
	ID3D12Resource* Resource() const noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE RTV() const noexcept;
	D3D12_RESOURCE_STATES State() const noexcept;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_CPU_DESCRIPTOR_HANDLE m_rtv{};
	D3D12_RESOURCE_STATES m_state = D3D12_RESOURCE_STATE_COMMON;
};
} 