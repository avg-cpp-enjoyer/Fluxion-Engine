#pragma once

#include <fluxion/renderer/graph/RenderGraph.hpp>
#include <fluxion/renderer/RenderContext.hpp>
#include <fluxion/renderer/GBuffer.hpp>
#include <fluxion/renderer/ConstantBuffer.hpp>
#include <fluxion/renderer/ConstantBuffers.hpp>

namespace fluxion {
class LightingPass : public RGPass {
public:
	LightingPass(GBuffer* gbuffer, RenderContext* ctx);
	~LightingPass() override = default;

	bool Initialize(ID3D12Device* device);
	void SetOutput(RGResource* outputRG, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV);
	void Setup(RGPassBuilder& builder) override;
	void Execute(ID3D12GraphicsCommandList* cmdList) override;
	constexpr const char* GetName() const override { return "LightingPass"; }
private:
	GBuffer* m_gbuffer;
	RenderContext* m_ctx;
	RGResource* m_outputRG = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_outputRTV{};
	ConstantBuffer<LightingParams> m_paramsBuffer;
};
}