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

struct FLUXION_ALIGNAS_CONSTANT_BUFFER GridParams {
	float cellSize = 1.0f;
	float lineWidth = 0.02f;
	float baseAlpha = 0.5f;
	float padding1 = 0.0f;
	DirectX::XMFLOAT3 gridColor = { 0.5f, 0.5f, 0.5f };
	float padding2 = 0.0f;
	DirectX::XMFLOAT3 axisXColor = { 1.0f, 0.0f, 0.0f };
	float padding3 = 0.0f;
	DirectX::XMFLOAT3 axisZColor = { 0.0f, 0.0f, 1.0f }; 
	float padding4 = 0.0f;
	float axisWidth = 0.04f;
	float fadeDistance = 50.0f;
	float padding5[2] = { 0.0f, 0.0f };
};

struct FLUXION_ALIGNAS_CONSTANT_BUFFER MaterialBuffer {
	DirectX::XMFLOAT4 baseColor; 
	uint32 textureIndex;         
	uint32 padding[3];           
};

struct FLUXION_ALIGNAS_CONSTANT_BUFFER LightingParams {
	uint32 albedoIndex;
	uint32 normalIndex;
	uint32 materialIndex;
	uint32 padding;
};
}