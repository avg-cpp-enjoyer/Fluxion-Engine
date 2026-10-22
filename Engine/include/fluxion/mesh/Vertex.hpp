#pragma once

#include <fluxion/common/Common.hpp>

#include <cstdint>
#include <vector>
#include <DirectXMath.h>

namespace fluxion {
struct Vertex {
	Vertex();
	Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 uv, DirectX::XMFLOAT4 color);
	Vertex(DirectX::XMVECTOR pos, DirectX::XMVECTOR normal, DirectX::XMVECTOR uv, DirectX::XMVECTOR color);

	Vertex& operator*=(const DirectX::XMMATRIX& matrix);
	friend Vertex operator*(Vertex vertex, const DirectX::XMMATRIX& matrix);

	Vertex& TransformPosition(const DirectX::XMMATRIX& matrix);
	Vertex& TransformNormal(const DirectX::XMMATRIX& matrix);
	DirectX::XMVECTOR PositionVec() const noexcept;
	DirectX::XMVECTOR NormalVec() const noexcept;
	DirectX::XMVECTOR UVVec() const noexcept;
	DirectX::XMVECTOR ColorVec() const noexcept;
	void SetPosition(DirectX::XMVECTOR vec) noexcept;
	void SetNormal(DirectX::XMVECTOR vec) noexcept;
	void SetUV(DirectX::XMVECTOR vec) noexcept;
	void SetColor(DirectX::XMVECTOR vec) noexcept;

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT4 color;
};

struct MeshData {
	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
};
} 