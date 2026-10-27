#pragma once
#include <fluxion/common/Common.hpp>
#include <DirectXMath.h>

namespace fluxion {
struct Material {
	uint32 textureIndex = 0;
	uint32 normalIndex = 0;
	uint32 roughnessIndex = 0;     
	uint32 metallicIndex = 0; 
	uint32 aoIndex = 0;
	DirectX::XMFLOAT4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};
} 