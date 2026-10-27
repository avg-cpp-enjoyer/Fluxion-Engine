#include <fluxion/renderer/GBuffer.hpp>
#include <fluxion/core/AssetManager.hpp>
#include <directx/d3dx12.h>

namespace fluxion {
bool GBuffer::Initialize(ID3D12Device* device, SRVHeap* srvHeap, uint32 width, uint32 height) {
	FLUXION_ENSURE_RET(device && srvHeap, false);

	if (!m_initialized) {
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
		rtvHeapDesc.NumDescriptors = 3;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		FLUXION_CHECK(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	}

	uint32 rtvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	m_albedoRTV = rtvHandle;

	FLUXION_ENSURE_RET(CreateGBufferTexture(device, srvHeap, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
		m_albedoResource, m_albedoRTV, m_albedoIndex), false);

	m_albedoRG.emplace(m_albedoResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	rtvHandle.ptr += rtvSize;
	m_normalRTV = rtvHandle;

	FLUXION_ENSURE_RET(CreateGBufferTexture(device, srvHeap, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
		m_normalResource, m_normalRTV, m_normalIndex), false);

	m_normalRG.emplace(m_normalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	rtvHandle.ptr += rtvSize;
	m_materialRTV = rtvHandle;

	FLUXION_ENSURE_RET(CreateGBufferTexture(device, srvHeap, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
		m_materialResource, m_materialRTV, m_materialIndex), false);

	m_materialRG.emplace(m_materialResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_initialized = true;
	return true;
}

void GBuffer::Shutdown() {
	m_albedoRG.reset();
	m_normalRG.reset();
	m_materialRG.reset();
	m_rtvHeap.Reset();
}

D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetAlbedoRTV() const {
	return m_albedoRTV;
}

D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetNormalRTV() const {
	return m_normalRTV;
}

D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetMaterialRTV() const {
	return m_materialRTV;
}

uint32 GBuffer::GetAlbedoIndex() const {
	return m_albedoIndex;
}

uint32 GBuffer::GetNormalIndex() const {
	return m_normalIndex;
}

uint32 GBuffer::GetMaterialIndex() const {
	return m_materialIndex;
}

RGResource* GBuffer::GetAlbedoRG() const {
	return m_albedoRG.has_value() ? const_cast<RGResource*>(&m_albedoRG.value()) : nullptr;
}

RGResource* GBuffer::GetNormalRG() const {
	return m_normalRG.has_value() ? const_cast<RGResource*>(&m_normalRG.value()) : nullptr;
}

RGResource* GBuffer::GetMaterialRG() const { 
	return m_materialRG.has_value() ? const_cast<RGResource*>(&m_materialRG.value()) : nullptr; 
}

bool GBuffer::CreateGBufferTexture(ID3D12Device* device, SRVHeap* srvHeap, uint32 width, uint32 height, DXGI_FORMAT format, 
	Microsoft::WRL::ComPtr<ID3D12Resource>& outResource, D3D12_CPU_DESCRIPTOR_HANDLE& outRTV, uint32& outSRVIndex) {
	D3D12_RESOURCE_DESC texDesc{};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	FLUXION_CHECK(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&outResource)));

	device->CreateRenderTargetView(outResource.Get(), nullptr, outRTV);
	if (outSRVIndex == UINT32_MAX) {
		outSRVIndex = srvHeap->AllocateSlot();
	}
	
	srvHeap->WriteTexture2D(outSRVIndex, outResource.Get(), format);
	return true;
}
}