#include <fluxion/renderer/Camera.hpp>
#include <Windows.h>

#include <algorithm>

namespace fluxion {
Camera::Camera() {
	m_position = { 0.0f, 0.0f, -5.0f };
	m_up = { 0.0f, 1.0f, 0.0f };  
	m_fov = 90.0f;
	m_aspectRatio = 16.0f / 9.0f;
	m_nearPlane = 0.1f;
	m_farPlane = 1000.0f;
}

void Camera::AddYawPitch(float32 yawDelta, float32 pitchDelta) {
	m_yaw += yawDelta;
	m_pitch -= pitchDelta;
	constexpr float maxPitch = DirectX::XMConvertToRadians(89.0f);
	m_pitch = std::clamp(m_pitch, -maxPitch, maxPitch);
}

void Camera::MoveLocal(float32 forwardAmout, float32 rightAmount, float32 upAmount, float32 distance) {
	DirectX::XMVECTOR forward = DirectX::XMVector3NormalizeEst(GetForwardVector());
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMVECTOR right = DirectX::XMVector3NormalizeEst(DirectX::XMVector3Cross(up, forward));

	DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_position);
	pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(forward, forwardAmout * distance));
	pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(right, rightAmount * distance));
	pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(up, upAmount * distance));
	DirectX::XMStoreFloat3(&m_position, pos);
}

void Camera::SetPosition(const DirectX::XMFLOAT3& position) {
	m_position = position;
}

void Camera::SetAspectRatio(float32 aspect) {
	m_aspectRatio = aspect;
}

DirectX::XMMATRIX Camera::GetViewMatrix() const {
	DirectX::XMVECTOR forward = GetForwardVector();
	DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&m_position);
	DirectX::XMVECTOR focus = DirectX::XMVectorAdd(eye, forward);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	return DirectX::XMMatrixLookAtLH(eye, focus, up);
}

DirectX::XMMATRIX Camera::GetProjectionMatrix() const {
	return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

DirectX::XMFLOAT3 Camera::GetPosition() const {
	return m_position;
}

DirectX::XMFLOAT4 Camera::GetPositionFloat4() const {
	return DirectX::XMFLOAT4(m_position.x, m_position.y, m_position.z, 1.0f);
}

DirectX::XMVECTOR Camera::GetForwardVector() const {
	return DirectX::XMVectorSet(
		sinf(m_yaw) * cosf(m_pitch), sinf(m_pitch),
		cosf(m_yaw) * cosf(m_pitch), 0.0f
	);
}
}
