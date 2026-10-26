#include <fluxion/renderer/passes/GridPass.hpp>

namespace fluxion {
GridPass::GridPass(RenderContext* ctx, D3D12_CPU_DESCRIPTOR_HANDLE depthDsv) : m_ctx(ctx), m_depthDsv(depthDsv) {}

void GridPass::SetOutput(RGResource* outputRG, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV) {
	m_outputRG = outputRG;
	m_outputRTV = outputRTV;
}

void GridPass::Setup(RGPassBuilder& builder) {
	builder.Write(m_outputRG);
}

void GridPass::Execute(ID3D12GraphicsCommandList* cmdList) {
	cmdList->SetPipelineState(PipelineManager::Get(PipelineMode::Grid));
	cmdList->OMSetRenderTargets(1, &m_outputRTV, FALSE, &m_depthDsv);

	TransformBuffer transform{};
	DirectX::XMStoreFloat4x4(&transform.world, DirectX::XMLoadFloat4x4(&m_ctx->gridItem->worldMatrix));
	DirectX::XMStoreFloat4x4(&transform.view, m_ctx->camera->GetViewMatrix());
	DirectX::XMStoreFloat4x4(&transform.projection, m_ctx->camera->GetProjectionMatrix());

	auto transformAddr = m_ctx->transformBuffer->AllocateAndCopy(&transform, sizeof(TransformBuffer));
	cmdList->SetGraphicsRootConstantBufferView(0, transformAddr);
	cmdList->SetGraphicsRootConstantBufferView(2, m_ctx->gridParamsBuffer->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(3, m_ctx->cameraBuffer->GetGPUVirtualAddress());
	m_ctx->gridItem->mesh->Draw(cmdList);
}

constexpr const char* GridPass::GetName() const {
	return "GridPass";
}
}
