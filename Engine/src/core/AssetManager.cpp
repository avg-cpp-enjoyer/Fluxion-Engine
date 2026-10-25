#include <fluxion/core/AssetManager.hpp>
#include <fluxion/mesh/MeshLoader.hpp>
#include <fluxion/core/GraphicsDevice.hpp>

namespace fluxion {
bool AssetManager::Initialize(ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12CommandAllocator* allocator) {
	m_device = device;
	m_queue = queue;
	m_allocator = allocator;

	m_memory = std::make_unique<std::byte[]>(MiB(16));
	m_arena.emplace(m_memory.get(), MiB(16));
	m_meshes.emplace(&m_arena.value());
	m_textures.emplace(&m_arena.value());
	m_materials.emplace(&m_arena.value());

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
	srvHeapDesc.NumDescriptors = kMaxTextures;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	FLUXION_CHECK(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
	m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return true;
}

void AssetManager::Shutdown() {
	m_meshes.reset();
	m_textures.reset();
	m_materials.reset();
	m_arena.reset();
	m_memory.reset();
	m_srvHeap.Reset();
}

bool AssetManager::LoadMeshes(const std::filesystem::path& directory) {
	FLUXION_ENSURE_RET(m_meshes.has_value(), false);
	FLUXION_ENSURE_RET(std::filesystem::exists(directory), false);

	uint32 loadedCount = 0;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
		if (!entry.is_regular_file()) {
			continue;
		}

		std::string ext = entry.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](uint8 c) -> int32 { return std::tolower(c); });
		if (ext != ".obj") {
			continue;
		}

		std::string meshName = entry.path().stem().string();
		if (LoadMesh(meshName, entry.path())) {
			loadedCount++;
		}
	}

	OutputDebugStringA(("[AssetManager] Loaded " + std::to_string(loadedCount) + " meshes.\n").c_str());
	return loadedCount > 0;
}

bool AssetManager::LoadTextures(const std::filesystem::path& directory) {
	FLUXION_ENSURE_RET(m_textures.has_value(), false);
	FLUXION_ENSURE_RET(std::filesystem::exists(directory), false);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
	FLUXION_CHECK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator, nullptr, IID_PPV_ARGS(&cmdList)));

	uint32 loadedCount = 0;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
		if (!entry.is_regular_file()) {
			continue;
		}

		std::string ext = entry.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](uint8 c) -> int32 { return std::tolower(c); });
		if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".dds") {
			continue;
		}

		std::string texName = entry.path().stem().string();
		if (LoadTexture(texName, entry.path(), cmdList.Get())) {
			loadedCount++;
		}
	}

	cmdList->Close();
	ID3D12CommandList* lists[] = { cmdList.Get() };
	m_queue->ExecuteCommandLists(1, lists);
	GraphicsDevice::WaitForGPU();
	OutputDebugStringA(("[AssetManager] Loaded " + std::to_string(loadedCount) + " textures.\n").c_str());
}

bool AssetManager::CreateGridMesh(const std::string& name) {
	FLUXION_ENSURE_RET(m_meshes.has_value(), false);

	auto makeVertex = [](float x, float z) -> Vertex {
		return Vertex(
			DirectX::XMFLOAT3{ x, 0.0f, z },
			DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f },         
			DirectX::XMFLOAT2{ 0.0f, 0.0f },               
			DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f }   
		);
	};

	std::vector<uint32> indices = { 0, 1, 2, 0, 2, 3 };
	std::vector<Vertex> verts = {
		makeVertex(-100.0f, -100.0f),
		makeVertex(-100.0f,  100.0f),
		makeVertex(100.0f,  100.0f),
		makeVertex(100.0f, -100.0f)
	};

	auto [it, inserted] = m_meshes->try_emplace(name);
	FLUXION_ENSURE_RET(inserted, false);

	MeshData data{ verts, indices };
	if (!it->second.Initialize(m_device, data)) {
		OutputDebugStringA(("[AssetManager] Failed to initialize grid mesh buffers: " + name + "\n").c_str());
		m_meshes->erase(it); 
		return false;
	}

	return true;
}

Mesh* AssetManager::GetMeshByName(const std::string& name) {
	FLUXION_ENSURE_RET(m_meshes.has_value(), nullptr);
	auto it = m_meshes->find(name);
	return it != m_meshes->end() ? &it->second : nullptr;
}

uint32 AssetManager::GetTextureIndexByTextureName(const std::string& name) {
	FLUXION_ENSURE_RET(m_textures.has_value(), 0);
	auto it = m_textures->find(name);
	return it != m_textures->end() ? it->second.GetIndex() : 0;
}

Material* AssetManager::GetMaterialByName(const std::string& name) {
	FLUXION_ENSURE_RET(m_materials.has_value(), nullptr);
	auto it = m_materials->find(name);
	return it != m_materials->end() ? &it->second : nullptr;
}

bool AssetManager::CreateMaterial(const std::string& name) {
	FLUXION_ENSURE_RET(m_materials.has_value(), false);

	Material mat;
	mat.baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	mat.textureIndex = 0;

	(*m_materials)[name] = mat;
	return true;
}

void AssetManager::AssignTextureToMaterial(Material& material, const std::string& textureName) {
	FLUXION_ENSURE(m_textures.has_value());

	auto texIt = m_textures->find(textureName);
	if (texIt == m_textures->end()) {
		OutputDebugStringA(("[AssetManager] Texture not found: " + textureName + "\n").c_str());
		return;
	}

	material.textureIndex = texIt->second.GetIndex();
}

ID3D12DescriptorHeap* AssetManager::GetSrvHeap() {
	return m_srvHeap.Get();
}

bool AssetManager::LoadMesh(const std::string& name, const std::filesystem::path& directory) {
	MeshData data;
	FLUXION_ENSURE_RET(MeshLoader::LoadFromOBJ(directory.string(), data), false);
	
	auto [it, inserted] = m_meshes->try_emplace(name);
	FLUXION_ENSURE_RET(inserted, false);

	if (!it->second.Initialize(m_device, data)) {
		OutputDebugStringA(("[AssetManager] Failed to initialize mesh buffers: " + name + "\n").c_str());
		m_meshes->erase(it); 
		return false;
	}

	return true;
}

bool AssetManager::LoadTexture(const std::string& name, const std::filesystem::path& directory, ID3D12GraphicsCommandList* cmdList) {
	FLUXION_ENSURE_RET(m_currentSrvIndex < kMaxTextures, false);

	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	srvCpuHandle.ptr += m_currentSrvIndex * m_srvDescriptorSize;

	auto [it, inserted] = m_textures->try_emplace(name);
	FLUXION_ENSURE_RET(inserted, false);

	if (!it->second.Initialize(m_device, cmdList, directory, srvCpuHandle, m_currentSrvIndex)) {
		m_textures->erase(it);
		return false;
	}

	m_currentSrvIndex++;
	return true;
}
}