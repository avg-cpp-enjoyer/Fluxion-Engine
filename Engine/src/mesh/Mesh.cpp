#include <fluxion/mesh/Mesh.hpp>
#include <directx/d3dx12.h>

namespace fluxion {
bool Mesh::Initialize(ID3D12Device* device, const MeshData& data) {
	FLUXION_ENSURE_RET(device && !data.vertices.empty() && !data.indices.empty(), false);
	m_indexCount = static_cast<uint32>(data.indices.size());

	auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	uint32 vbSize = static_cast<uint32>(data.vertices.size() * sizeof(Vertex));
	auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);
	FLUXION_CHECK(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer)));

	void* vbData = nullptr;
	m_vertexBuffer->Map(0, nullptr, &vbData);
	memcpy(vbData, data.vertices.data(), vbSize);
	m_vertexBuffer->Unmap(0, nullptr);

	m_vbView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = vbSize;
	m_vbView.StrideInBytes = sizeof(Vertex);

	uint32 ibSize = static_cast<uint32>(data.indices.size() * sizeof(uint32));
	auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(ibSize);
	FLUXION_CHECK(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &ibDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexBuffer)));

	void* ibData = nullptr;
	m_indexBuffer->Map(0, nullptr, &ibData);
	memcpy(ibData, data.indices.data(), ibSize);
	m_indexBuffer->Unmap(0, nullptr);

	m_ibView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_ibView.SizeInBytes = ibSize;
	m_ibView.Format = DXGI_FORMAT_R32_UINT;
	return true;
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) const {
	cmdList->IASetVertexBuffers(0, 1, &m_vbView);
	cmdList->IASetIndexBuffer(&m_ibView);
	cmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}
}