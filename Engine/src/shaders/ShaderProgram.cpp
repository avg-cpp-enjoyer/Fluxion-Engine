#include <fluxion/shaders/ShaderProgram.hpp>

namespace fluxion {
void ShaderProgram::AddShader(const wchar* path, const char* entryPoint, ShaderType type) {
	Shader shader;
	shader.CompileFromFile(path, entryPoint, type);
	m_shaders[shader.Name()] = std::move(shader);
}

Shader* ShaderProgram::GetShaderByName(const std::wstring& name) {
	auto it = m_shaders.find(name);
	return (it != m_shaders.end()) ? &it->second : nullptr;
}
}