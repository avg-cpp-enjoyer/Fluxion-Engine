#pragma once

#include <fluxion/common/Common.hpp>
#include <d3d12.h>
#include <wrl/client.h>
#include <filesystem>

namespace fluxion {
class Texture {
public:
	bool Initialize(
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		const std::filesystem::path& path, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, uint32 srvIndex
	);
	uint32 GetWidth() const;
	uint32 GetHeight() const;
	uint32 GetIndex() const;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
	uint32 m_width = 0;
	uint32 m_height = 0;
	uint32 m_srvIndex = 0;
};
} 