#pragma once

#include <fluxion/shaders/Shader.hpp>

#include <unordered_map>
#include <string>

namespace fluxion {
class ShaderProgram {
public:
	static void AddShader(const wchar* path, const char* entryPoint, ShaderType type);
	static Shader* GetShaderByName(const std::wstring& name);
private:
	static inline std::unordered_map<std::wstring, Shader> m_shaders;
};
}