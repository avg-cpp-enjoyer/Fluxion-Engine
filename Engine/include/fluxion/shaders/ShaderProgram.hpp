#pragma once

#include <fluxion/shaders/Shader.hpp>

#include <unordered_map>
#include <string>
#include <filesystem>

namespace fluxion {
class ShaderProgram {
public:
	static bool AddShader(const std::filesystem::path& filePath);
	static Shader* GetShaderByName(const std::wstring& name);
private:
	static inline std::unordered_map<std::wstring, Shader> m_shaders;
};
}