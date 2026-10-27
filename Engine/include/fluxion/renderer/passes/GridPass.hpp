#pragma once

#include <fluxion/renderer/graph/RenderGraph.hpp>
#include <fluxion/renderer/RenderContext.hpp>

namespace fluxion {
class GridPass : public RGPass {
public:
	GridPass(RenderContext* ctx, D3D12_CPU_DESCRIPTOR_HANDLE depthDsv);
	~GridPass() override = default;

	void SetOutput(RGResource* outputRG, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV);
	void Setup(RGPassBuilder& builder) override;
	void Execute(ID3D12GraphicsCommandList* cmdList) override;
	constexpr const char* GetName() const override;
private:
	RenderContext* m_ctx;
	RGResource* m_outputRG = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_outputRTV{};
	D3D12_CPU_DESCRIPTOR_HANDLE m_depthDsv;
};
}