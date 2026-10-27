#include <fluxion/renderer/passes/GeometryPass.hpp>

namespace fluxion {
GeometryPass::GeometryPass(GBuffer* gbuffer, RenderContext* ctx, D3D12_CPU_DESCRIPTOR_HANDLE depthDsv) : 
	m_gbuffer(gbuffer), m_ctx(ctx), m_depthDsv(depthDsv)
{}

void GeometryPass::Setup(RGPassBuilder& builder) {
	builder.Write(m_gbuffer->GetAlbedoRG());
	builder.Write(m_gbuffer->GetNormalRG());
	builder.Write(m_gbuffer->GetMaterialRG());
	builder.WriteDepth(m_ctx->depthRG);
}

void GeometryPass::Execute(ID3D12GraphicsCommandList* cmdList) {
	cmdList->SetPipelineState(PipelineManager::Get(PipelineMode::Geometry));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		m_gbuffer->GetAlbedoRTV(),
		m_gbuffer->GetNormalRTV(),
		m_gbuffer->GetMaterialRTV()
	};

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdList->OMSetRenderTargets(3, rtvs, FALSE, &m_depthDsv);
	cmdList->ClearRenderTargetView(m_gbuffer->GetAlbedoRTV(), clearColor, 0, nullptr);
	cmdList->ClearRenderTargetView(m_gbuffer->GetNormalRTV(), clearColor, 0, nullptr);
	cmdList->ClearRenderTargetView(m_gbuffer->GetMaterialRTV(), clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(m_depthDsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	DirectX::XMMATRIX view = m_ctx->camera->GetViewMatrix();
	DirectX::XMMATRIX proj = m_ctx->camera->GetProjectionMatrix();

	for (const auto& item : *m_ctx->renderItems) {
		TransformBuffer transform{};
		DirectX::XMStoreFloat4x4(&transform.world, DirectX::XMLoadFloat4x4(&item.worldMatrix));
		DirectX::XMStoreFloat4x4(&transform.view, view);
		DirectX::XMStoreFloat4x4(&transform.projection, proj);

		auto transformAddr = m_ctx->transformBuffer->AllocateAndCopy(&transform, sizeof(TransformBuffer));
		cmdList->SetGraphicsRootConstantBufferView(0, transformAddr);

		MaterialBuffer matData{};
		matData.textureIndex = item.material->textureIndex;
		matData.baseColor = item.material->baseColor;

		auto matAddr = m_ctx->materialBuffer->AllocateAndCopy(&matData, sizeof(MaterialBuffer));
		cmdList->SetGraphicsRootConstantBufferView(4, matAddr);
		cmdList->SetGraphicsRootConstantBufferView(3, m_ctx->cameraBuffer->GetGPUVirtualAddress());
		item.mesh->Draw(cmdList);
	}
}

constexpr const char* GeometryPass::GetName() const {
	return "GeometryPass";
}
}