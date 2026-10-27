#include <fluxion/renderer/Renderer.hpp>
#include <fluxion/core/GraphicsDevice.hpp>
#include <fluxion/shaders/ShaderProgram.hpp>
#include <fluxion/renderer/PipelineManager.hpp>
#include <fluxion/mesh/MeshLoader.hpp>
#include <fluxion/mesh/Mesh.hpp>
#include <fluxion/core/AssetManager.hpp>
#include <directx/d3dx12.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <filesystem>

#pragma warning(disable: 26495)

namespace fluxion {
Renderer::Renderer(uint32 width, uint32 height) : m_width(width), m_height(height) {}

Renderer::~Renderer() {
	Shutdown();
}

bool Renderer::Initialize(platform::win32::WindowHandle hwnd) {
	FLUXION_ENSURE_RET(!m_initialized, false);
	FLUXION_ENSURE_RET(hwnd, false);
	FLUXION_ENSURE_RET(m_width > 0 && m_height > 0, false);

	FLUXION_ENSURE_RET(GraphicsDevice::Device(), false);
	m_device = GraphicsDevice::Device();
	m_queue = GraphicsDevice::CommandQueue();

	FLUXION_ENSURE_RET(m_srvHeap.Initialize(m_device), false);
	FLUXION_ENSURE_RET(LoadShaders(), false);
	FLUXION_ENSURE_RET(CreateSwapChain(hwnd), false);
	FLUXION_ENSURE_RET(CreateRTVHeap(), false);
	FLUXION_ENSURE_RET(CreateRenderTargets(), false);
	FLUXION_ENSURE_RET(CreateCommandObjects(), false);
	FLUXION_ENSURE_RET(CreateFence(), false);
	FLUXION_ENSURE_RET(CreateDepthBuffer(), false);
	m_depthRG.emplace(m_depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	FLUXION_ENSURE_RET(PipelineManager::Initialize(m_device), false);
	FLUXION_ENSURE_RET(AssetManager::Initialize(m_device, m_queue, m_commandAllocators[0].Get(), &m_srvHeap), false);
	FLUXION_ENSURE_RET(CreateBuffers(), false);
	FLUXION_ENSURE_RET(m_gbuffer.Initialize(m_device, &m_srvHeap, m_width, m_height), false);

	std::filesystem::path assetsDir = FLUXION_ASSET_DIR;
	AssetManager::LoadMeshes(assetsDir / "models");
	AssetManager::LoadTextures(assetsDir / "textures");
	AssetManager::CreateGridMesh("grid");

	AssetManager::CreateMaterial("default_mat");
	AssetManager::CreateMaterial("cottage_mat");
	Material* cottageMat = AssetManager::GetMaterialByName("cottage_mat");
	AssetManager::AssignTextureToMaterial(*cottageMat, "Cottage_Clean_Base_Color");

	RenderItem cottage;
	cottage.mesh = AssetManager::GetMeshByName("cottage");
	cottage.material = cottageMat;
	cottage.pipelineMode = PipelineMode::Geometry;
	DirectX::XMStoreFloat4x4(&cottage.worldMatrix, DirectX::XMMatrixIdentity());
	m_renderItems.push_back(cottage);

	m_grid.mesh = AssetManager::GetMeshByName("grid");
	m_grid.material = AssetManager::GetMaterialByName("default_mat");
	m_grid.pipelineMode = PipelineMode::Grid;
	DirectX::XMStoreFloat4x4(&m_grid.worldMatrix, DirectX::XMMatrixIdentity());

	m_renderContext.camera = &m_camera;
	m_renderContext.gbuffer = &m_gbuffer;
	m_renderContext.renderItems = &m_renderItems;
	m_renderContext.transformBuffer = &m_transformBuffer;
	m_renderContext.materialBuffer = &m_materialBuffer;
	m_renderContext.cameraBuffer = &m_cameraBuffer;
	m_renderContext.gridParamsBuffer = &m_gridParamsBuffer;
	m_renderContext.lightBuffer = &m_lightBuffer;
	m_renderContext.gridItem = &m_grid;
	m_renderContext.depthRG = &m_depthRG.value();
	m_renderContext.width = m_width; 
	m_renderContext.height = m_height;

	m_geometryPass = std::make_unique<GeometryPass>(&m_gbuffer, &m_renderContext, m_dsv);
	m_lightingPass = std::make_unique<LightingPass>(&m_gbuffer, &m_renderContext);
	FLUXION_ENSURE_RET(m_lightingPass->Initialize(m_device), false);
	m_gridPass = std::make_unique<GridPass>(&m_renderContext, m_dsv);

	m_renderGraph.AddPass(m_geometryPass.get());
	m_renderGraph.AddPass(m_lightingPass.get());
	m_renderGraph.AddPass(m_gridPass.get());

	float32 width = static_cast<float32>(m_width);
	float32 height = static_cast<float32>(m_height);
	m_camera.SetAspectRatio(width / height);
	m_initialized = true;
	return true;
}

void Renderer::Resize(uint32 width, uint32 height) {
	FLUXION_ENSURE(m_initialized);
	FLUXION_ENSURE(width > 0 && height > 0);
	FLUXION_ENSURE(width != m_width && height != m_height);

	float32 newWidth = static_cast<float32>(width);
	float32 newHeight = static_cast<float32>(height);
	m_camera.SetAspectRatio(newWidth / newHeight);

	WaitForGPU();
	m_swapChainRG.reset();

	for (auto& rt : m_renderTargets) {
		rt.Shutdown();
	}

	DXGI_SWAP_CHAIN_DESC1 desc{};
	m_swapChain->GetDesc1(&desc);
	long result = m_swapChain->ResizeBuffers(kFrameCount, width, height, desc.Format, desc.Flags);
	FLUXION_ENSURE(SUCCEEDED(result));

	m_width = width;
	m_height = height;
	m_renderContext.width = m_width; 
	m_renderContext.height = m_height;

	FLUXION_ENSURE(CreateRenderTargets());
	FLUXION_ENSURE(CreateDepthBuffer());

	m_depthRG.reset();
	m_depthRG.emplace(m_depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_renderContext.depthRG = &m_depthRG.value();

	FLUXION_ENSURE(m_gbuffer.Initialize(m_device, &m_srvHeap, m_width, m_height));
	m_lightingPass->SetDepthIndex(m_depthSRVIndex);

	for (auto& fv : m_fenceValues) {
		fv = m_currentFenceValue;
	}
}

void Renderer::RenderFrame() {
	FLUXION_ENSURE(m_initialized);
	const uint32 frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValues[frameIndex]) {
		m_fence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_commandAllocators[frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[frameIndex].Get(), nullptr);

	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetGraphicsRootSignature(PipelineManager::GetRootSignature());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float32 widthViewport = static_cast<float32>(m_width);
	float32 heightViewport = static_cast<float32>(m_height);
	long widthScissor = static_cast<long>(m_width);
	long heightScissor = static_cast<long>(m_height);

	D3D12_VIEWPORT viewport{ 0.0f, 0.0f, widthViewport, heightViewport, 0.0f, 1.0f };
	D3D12_RECT scissorRect{ 0, 0, widthScissor, heightScissor };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	m_transformBuffer.Reset();
	m_materialBuffer.Reset();
	m_lightBuffer.Reset();

	auto& camData = m_cameraBuffer.GetData();
	camData.cameraPos = m_camera.GetPositionFloat4();
	DirectX::XMStoreFloat4x4(&camData.invViewProj, m_camera.GetInvViewProjMatrix());

	RenderTarget& rt = m_renderTargets[frameIndex];
	m_swapChainRG.reset();
	m_swapChainRG.emplace(rt.Resource(), D3D12_RESOURCE_STATE_PRESENT);

	m_lightingPass->SetOutput(&m_swapChainRG.value(), rt.RTV());
	m_lightingPass->SetDepthIndex(m_depthSRVIndex);
	m_gridPass->SetOutput(&m_swapChainRG.value(), rt.RTV());

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		rt.Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_renderGraph.Execute(m_commandList.Get());
	m_commandList->ResourceBarrier(1, &barrier);
	m_swapChainRG->SetState(D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->Close();

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_queue->ExecuteCommandLists(1, lists);
	m_swapChain->Present(1, 0);

	const uint64 fenceValue = ++m_currentFenceValue;
	m_queue->Signal(m_fence.Get(), fenceValue);
	m_fenceValues[frameIndex] = fenceValue;
}

void Renderer::Shutdown() {
	FLUXION_ENSURE(m_initialized);
	WaitForGPU();
	AssetManager::Shutdown();

	for (auto& rt : m_renderTargets) {
		rt.Shutdown();
	}

	if (m_fenceEvent) {
		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	m_rtvHeap.Reset();
	m_fence.Reset();
	m_commandList.Reset();

	for (auto& ca : m_commandAllocators) {
		ca.Reset();
	}

	m_device = nullptr;
	m_swapChain.Reset();
	m_initialized = false;
}

Camera& Renderer::GetCamera() {
	return m_camera;
}

bool Renderer::CreateSwapChain(platform::win32::WindowHandle hwnd) {
	IDXGIFactory6* factory = GraphicsDevice::DXGIFactory();
	FLUXION_ENSURE_RET(factory, false);

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Width = m_width;
	desc.Height = m_height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = kFrameCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	FLUXION_CHECK(factory->CreateSwapChainForHwnd(m_queue, hwnd, &desc, nullptr, nullptr, &swapChain1));
	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	FLUXION_CHECK(swapChain1.As(&m_swapChain));
	return true;
}

bool Renderer::CreateRTVHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = kFrameCount;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	FLUXION_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap)));
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	return true;
}

bool Renderer::CreateRenderTargets() {
	for (uint32 i = 0; i < kFrameCount; i++) {
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		FLUXION_CHECK(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)));
		auto& rt = m_renderTargets[i];
		FLUXION_ENSURE_RET(rt.Initialize(m_device, m_rtvHeap.Get(), i, m_rtvDescriptorSize, buffer.Get()), false);
	}

	return true;
}

bool Renderer::CreateCommandObjects() {
	for (uint32 i = 0; i < kFrameCount; i++) {
		FLUXION_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
	}

	auto* alloc = m_commandAllocators[0].Get();
	FLUXION_CHECK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc, nullptr, IID_PPV_ARGS(&m_commandList)));
	FLUXION_CHECK(m_commandList->Close());
	return true;
}

bool Renderer::CreateFence() {
	FLUXION_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	FLUXION_ENSURE_RET(m_fenceEvent, false);
	return true;
}

bool Renderer::CreateDepthBuffer() {
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	FLUXION_CHECK(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	m_dsv = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RESOURCE_DESC depthDesc{};
	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.Alignment = 0;
	depthDesc.Width = m_width;
	depthDesc.Height = m_height;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.MipLevels = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	FLUXION_CHECK(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_depthBuffer)));

	m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_dsv);
	if (m_depthSRVIndex == UINT32_MAX) {
		m_depthSRVIndex = m_srvHeap.AllocateSlot();
	}

	m_srvHeap.WriteTexture2D(m_depthSRVIndex, m_depthBuffer.Get(), DXGI_FORMAT_R32_FLOAT);
	return true;
}

bool Renderer::CreateBuffers() {
	FLUXION_ENSURE_RET(m_transformBuffer.Initialize(m_device, MiB(4)), false);
	FLUXION_ENSURE_RET(m_lightBuffer.Initialize(m_device, MiB(4)), false);
	FLUXION_ENSURE_RET(m_materialBuffer.Initialize(m_device, MiB(4)), false);
	FLUXION_ENSURE_RET(m_cameraBuffer.Initialize(m_device), false);
	FLUXION_ENSURE_RET(m_gridParamsBuffer.Initialize(m_device), false);
	m_gridParamsBuffer.GetData() = GridParams{};
	return true;
}

bool Renderer::LoadShaders() {
	std::filesystem::path shaderDir = std::filesystem::path(FLUXION_ASSET_DIR) / "shaders";
	FLUXION_ENSURE_RET(ShaderProgram::AddShader(shaderDir / L"GeometryPassVS.cso"), false);
	FLUXION_ENSURE_RET(ShaderProgram::AddShader(shaderDir / L"GeometryPassPS.cso"), false);
	FLUXION_ENSURE_RET(ShaderProgram::AddShader(shaderDir / L"LightingPassVS.cso"), false);
	FLUXION_ENSURE_RET(ShaderProgram::AddShader(shaderDir / L"LightingPassPhongPS.cso"), false);
	FLUXION_ENSURE_RET(ShaderProgram::AddShader(shaderDir / L"LightingPassPBRPS.cso"), false);
	FLUXION_ENSURE_RET(ShaderProgram::AddShader(shaderDir / L"GridVS.cso"), false);
	FLUXION_ENSURE_RET(ShaderProgram::AddShader(shaderDir / L"GridPS.cso"), false);
	return true;
}

void Renderer::WaitForGPU() {
	FLUXION_ENSURE(m_fence && m_fenceEvent);
	const uint64 fenceValue = ++m_currentFenceValue;
	m_queue->Signal(m_fence.Get(), fenceValue);

	if (m_fence->GetCompletedValue() < fenceValue) {
		m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}
}