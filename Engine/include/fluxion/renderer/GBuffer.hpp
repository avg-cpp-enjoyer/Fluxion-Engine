#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/renderer/graph/RGResource.hpp>
#include <fluxion/core/SRVHeap.hpp>

#include <d3d12.h>
#include <wrl/client.h>
#include <optional>

namespace fluxion {
class GBuffer {
public:
	bool Initialize(ID3D12Device* device, SRVHeap* srvHeap, uint32 width, uint32 height);
	void Shutdown();
	D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoRTV() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetNormalRTV() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetMaterialRTV() const;
	uint32 GetAlbedoIndex() const;
	uint32 GetNormalIndex() const;
	uint32 GetMaterialIndex() const;
	RGResource* GetAlbedoRG() const;
	RGResource* GetNormalRG() const;
	RGResource* GetMaterialRG() const;
private:
	static bool CreateGBufferTexture(
		ID3D12Device* device, SRVHeap* srvHeap, uint32 width, uint32 height,
		DXGI_FORMAT format, Microsoft::WRL::ComPtr<ID3D12Resource>& outResource,
		D3D12_CPU_DESCRIPTOR_HANDLE& outRTV, uint32& outSRVIndex
	);
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_albedoResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_normalResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_materialResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	uint32 m_albedoIndex = UINT32_MAX;
	uint32 m_normalIndex = UINT32_MAX;
	uint32 m_materialIndex = UINT32_MAX;
	D3D12_CPU_DESCRIPTOR_HANDLE m_albedoRTV{};
	D3D12_CPU_DESCRIPTOR_HANDLE m_normalRTV{};
	D3D12_CPU_DESCRIPTOR_HANDLE m_materialRTV{};
	std::optional<RGResource> m_albedoRG;
	std::optional<RGResource> m_normalRG;
	std::optional<RGResource> m_materialRG;
	bool m_initialized = false;
};
}