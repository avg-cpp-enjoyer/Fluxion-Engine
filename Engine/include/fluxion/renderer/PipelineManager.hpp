#pragma once

#include <fluxion/common/Common.hpp>
#include <d3d12.h>
#include <wrl/client.h>
#include <array>

namespace fluxion {
enum class PipelineMode {
	OpaqueSolid,
	Grid,
	Wireframe,   // TODO: Implement
	Transparent, // TODO: Implement
	DepthOnly,    // TODO: Implement
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
	static bool CreateOpaquePipeline(ID3D12Device* device);
	static bool CreateGridPipeline(ID3D12Device* device);
private:
	static inline Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	static inline std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<usize>(PipelineMode::Count)> m_pipelines;
};
} 