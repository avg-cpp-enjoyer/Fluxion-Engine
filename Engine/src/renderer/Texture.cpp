#include <fluxion/renderer/Texture.hpp>
#include <directx/d3dx12.h>

#include <DirectXTex.h>

namespace fluxion {
bool Texture::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, 
	const std::filesystem::path& path, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, uint32 srvIndex) {
	FLUXION_ENSURE_RET(device && cmdList, false);

	DirectX::ScratchImage image;
	FLUXION_CHECK(DirectX::LoadFromWICFile(path.wstring().c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image));

	const DirectX::TexMetadata& metadata = image.GetMetadata();
	m_width = static_cast<uint32>(metadata.width);
	m_height = static_cast<uint32>(metadata.height);

	D3D12_RESOURCE_DESC texDesc{};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.DepthOrArraySize = static_cast<uint16>(metadata.arraySize);
	texDesc.MipLevels = static_cast<uint16>(metadata.mipLevels);
	texDesc.Format = metadata.format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	FLUXION_CHECK(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_resource)));

	const uint64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, static_cast<uint64>(metadata.mipLevels));
	auto uploadProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	FLUXION_CHECK(device->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadBuffer)));

	std::vector<D3D12_SUBRESOURCE_DATA> subresources(metadata.mipLevels);
	for (usize i = 0; i < metadata.mipLevels; ++i) {
		const DirectX::Image* img = image.GetImage(0, i, 0);
		subresources[i].pData = img->pixels;
		subresources[i].RowPitch = img->rowPitch;
		subresources[i].SlicePitch = img->slicePitch;
	}

	UpdateSubresources(cmdList, m_resource.Get(), m_uploadBuffer.Get(), 0, 0, static_cast<uint32>(metadata.mipLevels), subresources.data());
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdList->ResourceBarrier(1, &barrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = static_cast<uint32>(metadata.mipLevels);
	device->CreateShaderResourceView(m_resource.Get(), &srvDesc, srvHandle);

	m_srvIndex = srvIndex;
	return true;
}

uint32 Texture::GetWidth() const {
	return m_width;
}

uint32 Texture::GetHeight() const {
	return m_height;
}

uint32 Texture::GetIndex() const {
	return m_srvIndex;
}
}
