#include <fluxion/renderer/PipelineManager.hpp>
#include <fluxion/shaders/ShaderProgram.hpp>
#include <fluxion/mesh/Vertex.hpp>
#include <directx/d3dx12.h>

namespace fluxion {
bool PipelineManager::Initialize(ID3D12Device* device) {
	FLUXION_ENSURE_RET(device, false);
	FLUXION_ENSURE_RET(CreateRootSignature(device), false);
	FLUXION_ENSURE_RET(CreateGeometryPipeline(device), false);
	FLUXION_ENSURE_RET(CreatePhongLightingPipeline(device), false);
	FLUXION_ENSURE_RET(CreatePBRLightingPipeline(device), false);
	FLUXION_ENSURE_RET(CreateGridPipeline(device), false);
	return true;
}

void PipelineManager::Shutdown() {
	m_rootSignature.Reset();
}

ID3D12PipelineState* PipelineManager::Get(PipelineMode mode) {
	return m_pipelines[static_cast<usize>(mode)].Get();
}

ID3D12RootSignature* PipelineManager::GetRootSignature() {
	return m_rootSignature.Get();
}

bool PipelineManager::CreateRootSignature(ID3D12Device* device) {
	D3D12_ROOT_PARAMETER rootParams[6]{};
	// 0: Transform (b0)
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[0].Descriptor.ShaderRegister = 0;
	rootParams[0].Descriptor.RegisterSpace = 0;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	// 1: Light (b1)
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[1].Descriptor.ShaderRegister = 1;
	rootParams[1].Descriptor.RegisterSpace = 0;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// 2: GridParams (b2)
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[2].Descriptor.ShaderRegister = 2;
	rootParams[2].Descriptor.RegisterSpace = 0;
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// 3: Camera (b3)
	rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[3].Descriptor.ShaderRegister = 3;
	rootParams[3].Descriptor.RegisterSpace = 0;
	rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	// 4: Material (b4)
	rootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[4].Descriptor.ShaderRegister = 4;
	rootParams[4].Descriptor.RegisterSpace = 0;
	rootParams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// 5: LightingParams (b5) 
	rootParams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[5].Descriptor.ShaderRegister = 5;
	rootParams[5].Descriptor.RegisterSpace = 0;
	rootParams[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// Static Sampler (s0)
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 1;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.NumParameters = 6;
	rootDesc.pParameters = rootParams;
	rootDesc.NumStaticSamplers = 1;
	rootDesc.pStaticSamplers = &sampler;
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

	Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
	FLUXION_CHECK(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

	void* bufPtr = signature->GetBufferPointer();
	usize bufSize = signature->GetBufferSize();
	FLUXION_CHECK(device->CreateRootSignature(0, bufPtr, bufSize, IID_PPV_ARGS(&m_rootSignature)));
	return true;
}

bool PipelineManager::CreateGeometryPipeline(ID3D12Device* device) {
	Shader* vs = ShaderProgram::GetShaderByName(L"GeometryPassVS");
	Shader* ps = ShaderProgram::GetShaderByName(L"GeometryPassPS");
	FLUXION_ENSURE_RET(vs && ps, false);

	D3D12_INPUT_ELEMENT_DESC layout[] = {
		FLUXION_INPUT_ELEMENT(Vertex, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, pos),
		FLUXION_INPUT_ELEMENT(Vertex, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, normal),
		FLUXION_INPUT_ELEMENT(Vertex, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, uv),
		FLUXION_INPUT_ELEMENT(Vertex, "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, color),
		FLUXION_INPUT_ELEMENT(Vertex, "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, tangent)
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
	psoDesc.NumRenderTargets = 3;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;      
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;  
	psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;     
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	FLUXION_CHECK(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
	m_pipelines[static_cast<usize>(PipelineMode::Geometry)] = pso;
	return true;
}

bool PipelineManager::CreateLightingPipeline(Shader* vs, Shader* ps, ID3D12Device* device, PipelineMode mode) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { nullptr, 0 };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vs->GetBlob()->GetBufferPointer(), vs->GetBlob()->GetBufferSize() };
	psoDesc.PS = { ps->GetBlob()->GetBufferPointer(), ps->GetBlob()->GetBufferSize() };

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	FLUXION_CHECK(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
	m_pipelines[static_cast<usize>(mode)] = pso;
	return true;
}

bool PipelineManager::CreatePhongLightingPipeline(ID3D12Device* device) {
	Shader* vs = ShaderProgram::GetShaderByName(L"LightingPassVS");
	Shader* ps = ShaderProgram::GetShaderByName(L"LightingPassPhongPS");
	FLUXION_ENSURE_RET(vs && ps, false);
	FLUXION_ENSURE_RET(CreateLightingPipeline(vs, ps, device, PipelineMode::LightingPhong), false);
	return true;
}

bool PipelineManager::CreatePBRLightingPipeline(ID3D12Device* device) {
	Shader* vs = ShaderProgram::GetShaderByName(L"LightingPassVS");
	Shader* ps = ShaderProgram::GetShaderByName(L"LightingPassPBRPS");
	FLUXION_ENSURE_RET(vs && ps, false);
	FLUXION_ENSURE_RET(CreateLightingPipeline(vs, ps, device, PipelineMode::LightingPBR), false);
	return true;
}

bool PipelineManager::CreateGridPipeline(ID3D12Device* device) {
	Shader* vs = ShaderProgram::GetShaderByName(L"GridVS");
	Shader* ps = ShaderProgram::GetShaderByName(L"GridPS");
	FLUXION_ENSURE_RET(vs && ps, false);

	D3D12_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { layout, _countof(layout) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vs->GetBlob()->GetBufferPointer(), vs->GetBlob()->GetBufferSize() };
	psoDesc.PS = { ps->GetBlob()->GetBufferPointer(), ps->GetBlob()->GetBufferSize() };

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; 
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	FLUXION_CHECK(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
	m_pipelines[static_cast<usize>(PipelineMode::Grid)] = pso;
	return true;
}
}
