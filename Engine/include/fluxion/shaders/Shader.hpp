#pragma once

#include <fluxion/common/Common.hpp>
#include <d3d12.h>
#include <wrl/client.h>
#include <string>

namespace fluxion {
enum class ShaderType {
	Vertex,
	Pixel,
	Compute
};

class Shader {
public:
	void CompileFromFile(const wchar* path, const char* entryPoint, ShaderType type);
	ID3DBlob* GetBlob() const;
	const std::wstring& Name() const;
private:
	Microsoft::WRL::ComPtr<ID3DBlob> m_blob;
	std::wstring m_name;
};
}