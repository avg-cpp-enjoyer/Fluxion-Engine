#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/mesh/Mesh.hpp>
#include <fluxion/renderer/Material.hpp>
#include <fluxion/renderer/PipelineManager.hpp>
#include <DirectXMath.h>

namespace fluxion {
struct RenderItem {
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	PipelineMode pipelineMode = PipelineMode::OpaqueSolid;
	DirectX::XMFLOAT4X4 worldMatrix;
};
} 