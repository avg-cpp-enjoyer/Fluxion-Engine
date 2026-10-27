#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/shaders/Shader.hpp>

#include <d3d12.h>
#include <wrl/client.h>
#include <array>

namespace fluxion {
enum class PipelineMode {
	Geometry,
	LightingPhong,
	LightingPBR,      
	Grid,
	Wireframe,   // TODO: Implement
	Transparent, // TODO: Implement
	DepthOnly,   // TODO: Implement
	Count
};

class PipelineManager {
public:
	static bool Initialize(ID3D12Device* device);
	static void Shutdown();
	static ID3D12PipelineState* Get(PipelineMode mode);
	static ID3D12RootSignature* GetRootSignature();
private:
	static bool CreateRootSignature(ID3D12Device* device);
	static bool CreateGeometryPipeline(ID3D12Device* device);
	static bool CreateLightingPipeline(Shader* vs, Shader* ps, ID3D12Device* device, PipelineMode mode);
	static bool CreatePhongLightingPipeline(ID3D12Device* device);
	static bool CreatePBRLightingPipeline(ID3D12Device* device);
	static bool CreateGridPipeline(ID3D12Device* device);
private:
	static inline Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	static inline std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<usize>(PipelineMode::Count)> m_pipelines;
};
} 