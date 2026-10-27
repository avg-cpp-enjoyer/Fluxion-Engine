#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/renderer/Texture.hpp>
#include <fluxion/mesh/Mesh.hpp>
#include <fluxion/renderer/Material.hpp>
#include <fluxion/core/SRVHeap.hpp>

#include <d3d12.h>
#include <wrl/client.h>
#include <filesystem>
#include <unordered_map>
#include <memory_resource>
#include <memory>
#include <string>
#include <optional>

namespace fluxion {
class AssetManager {
public:
	static bool Initialize(ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12CommandAllocator* allocator, SRVHeap* srvHeap);
	static void Shutdown();
	static bool LoadMeshes(const std::filesystem::path& directory);
	static bool LoadTextures(const std::filesystem::path& directory);
	static bool CreateGridMesh(const std::string& name);
	static Mesh* GetMeshByName(const std::string& name);
	static uint32 GetTextureIndexByTextureName(const std::string& name); 
	static Material* GetMaterialByName(const std::string& name); 
	static bool CreateMaterial(const std::string& name);
	static void AssignTextureToMaterial(Material& material, const std::string& textureName);
private:
	static bool LoadMesh(const std::string& name, const std::filesystem::path& directory);
	static bool LoadTexture(const std::string& name, const std::filesystem::path& directory, ID3D12GraphicsCommandList* cmdList);
private:
	static inline ID3D12Device* m_device = nullptr;
	static inline ID3D12CommandQueue* m_queue = nullptr;
	static inline ID3D12CommandAllocator* m_allocator = nullptr;
	static inline SRVHeap* m_srvHeap = nullptr;

	static inline std::unique_ptr<std::byte[]> m_memory;
	static inline std::optional<std::pmr::monotonic_buffer_resource> m_arena;
	static inline std::optional<std::pmr::unordered_map<std::string, Mesh>> m_meshes;
	static inline std::optional<std::pmr::unordered_map<std::string, Texture>> m_textures;
	static inline std::optional<std::pmr::unordered_map<std::string, Material>> m_materials;
};
} 