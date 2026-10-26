#pragma once

#include <fluxion/renderer/graph/RGPassBuilder.hpp>

namespace fluxion {
class RGPass {
public:
	virtual ~RGPass() = default;
	virtual void Setup(RGPassBuilder& builder) = 0;
	virtual void Execute(ID3D12GraphicsCommandList* cmdList) = 0;
	virtual constexpr const char* GetName() const = 0;
};
}