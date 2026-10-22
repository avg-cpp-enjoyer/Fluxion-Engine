#include <fluxion/mesh/Vertex.hpp>

namespace fluxion {
Vertex::Vertex() : pos(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), uv(0.0f, 0.0f), color(1.0f, 1.0f, 1.0f, 1.0f) {}

Vertex::Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 uv, DirectX::XMFLOAT4 color)
	: pos(pos), normal(normal), uv(uv), color(color)
{}

Vertex::Vertex(DirectX::XMVECTOR pos, DirectX::XMVECTOR normal, DirectX::XMVECTOR uv, DirectX::XMVECTOR color) {
	DirectX::XMStoreFloat3(&this->pos, pos);
	DirectX::XMStoreFloat3(&this->normal, normal);
	DirectX::XMStoreFloat2(&this->uv, uv);
	DirectX::XMStoreFloat4(&this->color, color);
}

Vertex& Vertex::operator*=(const DirectX::XMMATRIX& matrix) {
	SetPosition(DirectX::XMVector3Transform(PositionVec(), matrix));
	SetNormal(DirectX::XMVector3TransformNormal(NormalVec(), matrix));
	return *this;
}

Vertex operator*(Vertex vertex, const DirectX::XMMATRIX& matrix) {
	vertex *= matrix;
	return vertex;
}

Vertex& Vertex::TransformPosition(const DirectX::XMMATRIX& matrix) {
	SetPosition(DirectX::XMVector3Transform(PositionVec(), matrix));
	return *this;
}

Vertex& Vertex::TransformNormal(const DirectX::XMMATRIX& matrix) {
	SetNormal(DirectX::XMVector3TransformNormal(NormalVec(), matrix));
	return *this;
}

DirectX::XMVECTOR Vertex::PositionVec() const noexcept {
	return XMLoadFloat3(&pos);
}

DirectX::XMVECTOR Vertex::NormalVec() const noexcept {
	return XMLoadFloat3(&normal);
}

DirectX::XMVECTOR Vertex::UVVec() const noexcept {
	return XMLoadFloat2(&uv);
}

DirectX::XMVECTOR Vertex::ColorVec() const noexcept {
	return XMLoadFloat4(&color);
}

void Vertex::SetPosition(DirectX::XMVECTOR vec) noexcept {
	XMStoreFloat3(&pos, vec);
}

void Vertex::SetNormal(DirectX::XMVECTOR vec) noexcept {
	XMStoreFloat3(&normal, vec);
}

void Vertex::SetUV(DirectX::XMVECTOR vec) noexcept {
	XMStoreFloat2(&uv, vec);
}

void Vertex::SetColor(DirectX::XMVECTOR vec) noexcept {
	XMStoreFloat4(&color, vec);
}
}