#pragma once

#include <fluxion/renderer/graph/RenderGraph.hpp>
#include <fluxion/renderer/RenderContext.hpp>

namespace fluxion {
class GeometryPass : public RGPass {
public:
	GeometryPass(GBuffer* gbuffer, RenderContext* ctx, D3D12_CPU_DESCRIPTOR_HANDLE depthDsv);
	~GeometryPass() override = default;

	void Setup(RGPassBuilder& builder) override;
	void Execute(ID3D12GraphicsCommandList* cmdList) override;
	constexpr const char* GetName() const override;
private:
	GBuffer* m_gbuffer;
	RenderContext* m_ctx;
	D3D12_CPU_DESCRIPTOR_HANDLE m_depthDsv;
};
}