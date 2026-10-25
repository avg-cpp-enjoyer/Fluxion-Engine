#pragma once

#include <fluxion/common/Common.hpp>
#include <fluxion/renderer/Texture.hpp>
#include <fluxion/mesh/Mesh.hpp>
#include <fluxion/renderer/Material.hpp>

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
	static bool Initialize(ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12CommandAllocator* allocator);
	static void Shutdown();
	static bool LoadMeshes(const std::filesystem::path& directory);
	static bool LoadTextures(const std::filesystem::path& directory);
	static bool CreateGridMesh(const std::string& name);
	static Mesh* GetMeshByName(const std::string& name);
	static uint32 GetTextureIndexByTextureName(const std::string& name); 
	static Material* GetMaterialByName(const std::string& name); 
	static bool CreateMaterial(const std::string& name);
	static void AssignTextureToMaterial(Material& material, const std::string& textureName);
	static ID3D12DescriptorHeap* GetSrvHeap();
private:
	static bool LoadMesh(const std::string& name, const std::filesystem::path& directory);
	static bool LoadTexture(const std::string& name, const std::filesystem::path& directory, ID3D12GraphicsCommandList* cmdList);
private:
	static inline ID3D12Device* m_device = nullptr;
	static inline ID3D12CommandQueue* m_queue = nullptr;
	static inline ID3D12CommandAllocator* m_allocator = nullptr;
	static constexpr uint32 kMaxTextures = 256;
	static inline Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	static inline uint32 m_srvDescriptorSize = 0;
	static inline uint32 m_currentSrvIndex = 0;
	static inline std::unique_ptr<std::byte[]> m_memory;
	static inline std::optional<std::pmr::monotonic_buffer_resource> m_arena;
	static inline std::optional<std::pmr::unordered_map<std::string, Mesh>> m_meshes;
	static inline std::optional<std::pmr::unordered_map<std::string, Texture>> m_textures;
	static inline std::optional<std::pmr::unordered_map<std::string, Material>> m_materials;
};
} 