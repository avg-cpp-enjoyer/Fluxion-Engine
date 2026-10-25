#include <fluxion/shaders/Shader.hpp>

#include <wrl/client.h>
#include <vector>
#include <fstream>
#include <d3dcompiler.h>
#include <filesystem>

namespace fluxion {
bool Shader::LoadFromCSO(const std::filesystem::path& filePath) {
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	FLUXION_ENSURE_RET(file.is_open(), false);

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	FLUXION_CHECK(D3DCreateBlob(size, &m_blob));

	file.read(static_cast<char*>(m_blob->GetBufferPointer()), size);
	m_name = filePath.stem().wstring();
	return true;
}

ID3DBlob* Shader::GetBlob() const {
	return m_blob.Get(); 
}

const std::wstring& Shader::Name() const {
	return m_name;
}
}
