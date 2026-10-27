#pragma once

#include <fluxion/renderer/graph/RenderGraph.hpp>
#include <fluxion/renderer/RenderContext.hpp>
#include <fluxion/renderer/GBuffer.hpp>
#include <fluxion/renderer/ConstantBuffer.hpp>
#include <fluxion/renderer/ConstantBuffers.hpp>
#include <fluxion/renderer/PipelineManager.hpp>

namespace fluxion {
class LightingPass : public RGPass {
public:
	LightingPass(GBuffer* gbuffer, RenderContext* ctx);
	~LightingPass() override = default;

	bool Initialize(ID3D12Device* device);
	void SetOutput(RGResource* outputRG, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV);
	void Setup(RGPassBuilder& builder) override;
	void Execute(ID3D12GraphicsCommandList* cmdList) override;
	void UsePBR(bool usePBR);
	constexpr const char* GetName() const override;
	void SetDepthIndex(uint32 srvIndex);
private:
	GBuffer* m_gbuffer;
	RenderContext* m_ctx;
	RGResource* m_outputRG = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_outputRTV{};
	ConstantBuffer<LightingParams> m_paramsBuffer;
	PipelineMode m_mode = PipelineMode::LightingPBR;
};
}