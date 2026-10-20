#include <fluxion/renderer/RenderTarget.hpp>
#include <fluxion/common/Common.hpp>

#include <d3d12.h>

namespace fluxion {
bool RenderTarget::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* rtvHeap, uint32 rtvIndex, 
	uint32 rtvDescriptorSize, ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState) {
	FLUXION_ENSURE_RET(device, false);
	FLUXION_ENSURE_RET(rtvHeap, false);
	FLUXION_ENSURE_RET(resource, false);

	m_resource = resource;
	m_state = initialState;
	m_rtv = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	m_rtv.ptr += static_cast<usize>(rtvIndex) * rtvDescriptorSize;

	device->CreateRenderTargetView(m_resource.Get(), nullptr, m_rtv);
	return true;
}

void RenderTarget::Shutdown() {
	m_resource.Reset();
	m_rtv = {};
	m_state = D3D12_RESOURCE_STATE_COMMON;
}

void RenderTarget::Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES newState) {
	FLUXION_ENSURE(cmdList);
	FLUXION_ENSURE(m_resource);

	if (m_state == newState) {
		return;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_resource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = m_state;
	barrier.Transition.StateAfter = newState;

	cmdList->ResourceBarrier(1, &barrier);
	m_state = newState;
}

void RenderTarget::Clear(ID3D12GraphicsCommandList* cmdList, const float color[4]) {
	FLUXION_ENSURE(cmdList);
	FLUXION_ENSURE(m_resource);
	FLUXION_ENSURE(m_state == D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdList->ClearRenderTargetView(m_rtv, color, 0, nullptr);
}

ID3D12Resource* RenderTarget::Resource() const noexcept {
	return m_resource.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget::RTV() const noexcept {
	return m_rtv;
}

D3D12_RESOURCE_STATES RenderTarget::State() const noexcept {
	return m_state;
}
} 