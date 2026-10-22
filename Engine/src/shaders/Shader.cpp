#include <fluxion/shaders/Shader.hpp>

#include <filesystem>
#include <d3dcompiler.h>

namespace fluxion {
void Shader::CompileFromFile(const wchar* fileName, const char* entryPoint, ShaderType type) {
	auto profileToString = [](ShaderType type) -> const char* {
		switch (type) {
		case ShaderType::Vertex: return "vs_5_1";
		case ShaderType::Pixel: return "ps_5_1";
		case ShaderType::Compute: return "cs_5_1";
		default: return "vs_5_1";
		}
	};

	uint32 flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	std::filesystem::path path = std::filesystem::path(FLUXION_SHADER_DIR) / fileName;
	if (SUCCEEDED(D3DCompileFromFile(path.c_str(), nullptr, nullptr, entryPoint, profileToString(type), flags, 0, &m_blob, &errorBlob))) {
		m_name = fileName;
		return;
	}

	if (errorBlob) {
		OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
	}
}

ID3DBlob* Shader::GetBlob() const {
	return m_blob.Get(); 
}

const std::wstring& Shader::Name() const {
	return m_name;
}
}
