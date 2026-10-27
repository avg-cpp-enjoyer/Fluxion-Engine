#pragma once

#include <fluxion/common/Common.hpp>
#include <d3d12.h>
#include <wrl/client.h>
#include <filesystem>

namespace fluxion {
class Texture {
public:
	bool Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::filesystem::path& path);
	uint32 GetWidth() const;
	uint32 GetHeight() const;
	uint32 GetIndex() const;
	void SetIndex(uint32 index);
	ID3D12Resource* GetResource() const;
	DXGI_FORMAT GetFormat() const;
	uint32 GetMipLevels() const;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
	uint32 m_width = 0;
	uint32 m_height = 0;
	uint32 m_srvIndex = 0;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
	uint32 m_mipLevels = 0;
};
} 