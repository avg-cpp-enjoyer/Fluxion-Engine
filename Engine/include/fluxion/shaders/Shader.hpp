#pragma once

#include <fluxion/common/Common.hpp>

#include <Windows.h>
#include <unknwn.h>   
#include <wrl/client.h> 
#include <dxcapi.h>
#include <d3d12.h>
#include <string>
#include <filesystem>

namespace fluxion {
class Shader {
public:
	bool LoadFromCSO(const std::filesystem::path& filePath);
	ID3DBlob* GetBlob() const;
	const std::wstring& Name() const;
private:
	Microsoft::WRL::ComPtr<ID3DBlob> m_blob;
	std::wstring m_name;
};
}