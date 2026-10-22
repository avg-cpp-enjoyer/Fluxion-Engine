#pragma once

#include <fluxion/common/Common.hpp>

#include <DirectXMath.h>

namespace fluxion {
struct FLUXION_ALIGNAS_CONSTANT_BUFFER TransformBuffer {
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

struct FLUXION_ALIGNAS_CONSTANT_BUFFER LightBuffer {
	DirectX::XMFLOAT4 lightPos;   
	DirectX::XMFLOAT4 lightColor;
};

struct FLUXION_ALIGNAS_CONSTANT_BUFFER CameraParams {
	DirectX::XMFLOAT4 cameraPos;
};
}