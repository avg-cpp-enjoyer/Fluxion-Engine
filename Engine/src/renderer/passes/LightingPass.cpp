#include <fluxion/renderer/passes/LightingPass.hpp>

namespace fluxion {
LightingPass::LightingPass(GBuffer* gbuffer, RenderContext* ctx) : m_gbuffer(gbuffer), m_ctx(ctx) {}

bool LightingPass::Initialize(ID3D12Device* device) {
	return m_paramsBuffer.Initialize(device);
}

void LightingPass::SetOutput(RGResource* outputRG, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV) {
	m_outputRG = outputRG;
	m_outputRTV = outputRTV;
}

void LightingPass::Setup(RGPassBuilder& builder) {
	builder.Read(m_gbuffer->GetAlbedoRG());
	builder.Read(m_gbuffer->GetNormalRG());
	builder.Read(m_gbuffer->GetMaterialRG());
	builder.ReadDepth(m_ctx->depthRG);
	builder.Write(m_outputRG);
}

void LightingPass::Execute(ID3D12GraphicsCommandList* cmdList) {
	cmdList->SetPipelineState(PipelineManager::Get(m_mode));
	cmdList->OMSetRenderTargets(1, &m_outputRTV, FALSE, nullptr);

	auto& params = m_paramsBuffer.GetData();
	params.albedoIndex = m_gbuffer->GetAlbedoIndex();
	params.normalIndex = m_gbuffer->GetNormalIndex();
	params.materialIndex = m_gbuffer->GetMaterialIndex();
	params.screenWidth = static_cast<float>(m_ctx->width);
	params.screenHeight = static_cast<float>(m_ctx->height);

	LightBuffer light{};
	light.lightPos = { 100.0f, 50.0f, -5.0f, 1.0f }; 
	light.lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	auto lightAddr = m_ctx->lightBuffer->AllocateAndCopy(&light, sizeof(LightBuffer));
	cmdList->SetGraphicsRootConstantBufferView(1, lightAddr);

	cmdList->SetGraphicsRootConstantBufferView(3, m_ctx->cameraBuffer->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(5, m_paramsBuffer.GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(3, 1, 0, 0);
}

void LightingPass::UsePBR(bool usePBR) {
	m_mode = usePBR ? PipelineMode::LightingPBR : PipelineMode::LightingPhong;
}

constexpr const char* LightingPass::GetName() const {
	return "LightingPass";
}

void LightingPass::SetDepthIndex(uint32 srvIndex) {
	m_paramsBuffer.GetData().depthIndex = srvIndex;
}
}