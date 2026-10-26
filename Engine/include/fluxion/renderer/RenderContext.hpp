#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/renderer/Camera.hpp>
#include <fluxion/renderer/RenderItem.hpp>
#include <fluxion/renderer/GBuffer.hpp>
#include <fluxion/core/RingBuffer.hpp>
#include <fluxion/renderer/ConstantBuffer.hpp>
#include <fluxion/renderer/ConstantBuffers.hpp>

#include <vector>

namespace fluxion {
struct RenderContext {
	Camera* camera = nullptr;
	std::vector<RenderItem>* renderItems = nullptr;
	GBuffer* gbuffer = nullptr;
	RingBuffer* transformBuffer = nullptr;
	RingBuffer* materialBuffer = nullptr;
	ConstantBuffer<CameraParams>* cameraBuffer = nullptr;
	RingBuffer* lightBuffer = nullptr;
	ConstantBuffer<GridParams>* gridParamsBuffer = nullptr;
	RenderItem* gridItem = nullptr;
};
}