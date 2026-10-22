#include <fluxion/renderer/PipelineManager.hpp>
#include <fluxion/shaders/ShaderProgram.hpp>
#include <fluxion/mesh/Vertex.hpp>
#include <directx/d3dx12.h>

namespace fluxion {
bool PipelineManager::Initialize(ID3D12Device* device) {
	FLUXION_ENSURE_RET(device, false);
	FLUXION_ENSURE_RET(CreateRootSignature(device), false);
	FLUXION_ENSURE_RET(CreateOpaquePipeline(device), false);
	return true;
}

void PipelineManager::Shutdown() {
	m_pipelines.clear();
	m_rootSignature.Reset();
}

ID3D12PipelineState* PipelineManager::Get(PipelineMode mode) {
	auto it = m_pipelines.find(mode);
	if (it != m_pipelines.end()) {
		return it->second.Get();
	}

	return nullptr;
}

ID3D12RootSignature* PipelineManager::GetRootSignature() {
	return m_rootSignature.Get();
}

bool PipelineManager::CreateRootSignature(ID3D12Device* device) {
	D3D12_ROOT_PARAMETER rootParams[3]{};
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[0].Descriptor.ShaderRegister = 0;
	rootParams[0].Descriptor.RegisterSpace = 0;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[1].Descriptor.ShaderRegister = 1;
	rootParams[1].Descriptor.RegisterSpace = 0;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[2].Descriptor.ShaderRegister = 2;
	rootParams[2].Descriptor.RegisterSpace = 0;
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	D3D12_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.NumParameters = 3;
	rootDesc.pParameters = rootParams;
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	FLUXION_CHECK(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

	void* bufferPtr = signature->GetBufferPointer();
	usize bufferSize = signature->GetBufferSize();
	FLUXION_CHECK(device->CreateRootSignature(0, bufferPtr, bufferSize, IID_PPV_ARGS(&m_rootSignature)));
	return true;
}

bool PipelineManager::CreateOpaquePipeline(ID3D12Device* device) {
	auto* vs = ShaderProgram::GetShaderByName(L"VertexShader.hlsl");
	auto* ps = ShaderProgram::GetShaderByName(L"MeshOpaqueSolid.hlsl");
	FLUXION_ENSURE_RET(vs && ps, false);

	D3D12_INPUT_ELEMENT_DESC layout[] = {
		FLUXION_INPUT_ELEMENT(Vertex, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, pos),
		FLUXION_INPUT_ELEMENT(Vertex, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, normal),
		FLUXION_INPUT_ELEMENT(Vertex, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, uv),
		FLUXION_INPUT_ELEMENT(Vertex, "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, color)
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { layout, _countof(layout) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vs->GetBlob()->GetBufferPointer(), vs->GetBlob()->GetBufferSize() };
	psoDesc.PS = { ps->GetBlob()->GetBufferPointer(), ps->GetBlob()->GetBufferSize() };

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	FLUXION_CHECK(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
	m_pipelines[PipelineMode::OpaqueSolid] = pso;
	return true;
}
}
