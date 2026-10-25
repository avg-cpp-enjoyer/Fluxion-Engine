#include <fluxion/shaders/ShaderProgram.hpp>

namespace fluxion {
bool ShaderProgram::AddShader(const std::filesystem::path& filePath) {
	Shader shader;
	FLUXION_ENSURE_RET(shader.LoadFromCSO(filePath), false);
	m_shaders[shader.Name()] = std::move(shader);
	return true;
}

Shader* ShaderProgram::GetShaderByName(const std::wstring& name) {
	auto it = m_shaders.find(name);
	return (it != m_shaders.end()) ? &it->second : nullptr;
}
}