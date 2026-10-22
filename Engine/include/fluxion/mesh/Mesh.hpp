#pragma once

#include <fluxion/mesh/Vertex.hpp>
#include <fluxion/common/Common.hpp>

#include <d3d12.h>
#include <wrl/client.h>

namespace fluxion {
class Mesh {
public:
	bool Initialize(ID3D12Device* device, const MeshData& data);
	void Draw(ID3D12GraphicsCommandList* cmdList) const;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vbView{};
	D3D12_INDEX_BUFFER_VIEW m_ibView{};
	uint32 m_indexCount = 0;
};
}