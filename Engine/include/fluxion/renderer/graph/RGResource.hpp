#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace fluxion {
class RGResource {
public:
	RGResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialState);
	~RGResource() = default;

	ID3D12Resource* GetResource() const;
	D3D12_RESOURCE_STATES GetState() const;
	void SetState(D3D12_RESOURCE_STATES newState);
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_RESOURCE_STATES m_currentState;
};
}