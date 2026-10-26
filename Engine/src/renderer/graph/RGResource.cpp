#include <fluxion/renderer/graph/RGResource.hpp>

namespace fluxion {
RGResource::RGResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialState) :
	m_resource(resource), m_currentState(initialState)
{}

ID3D12Resource* RGResource::GetResource() const {
	return m_resource.Get();
}

D3D12_RESOURCE_STATES RGResource::GetState() const {
	return m_currentState;
}

void RGResource::SetState(D3D12_RESOURCE_STATES newState) {
	m_currentState = newState;
}
}
