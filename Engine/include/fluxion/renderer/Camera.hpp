#pragma once

#include <fluxion/common/Common.hpp>
#include <DirectXMath.h>

namespace fluxion {
class Camera {
public:
	Camera();
	~Camera() = default;

	void AddYawPitch(float32 yawDelta, float32 pitchDelta);
	void MoveLocal(float32 forward, float32 right, float32 up, float32 distance);
	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetAspectRatio(float32 aspect);
	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMMATRIX GetInvViewProjMatrix() const;
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT4 GetPositionFloat4() const;
private:
	DirectX::XMVECTOR GetForwardVector() const;
private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_up;
	float m_fov;
	float m_aspectRatio;  
	float m_nearPlane;
	float m_farPlane;
	float m_yaw = 0.0f; 
	float m_pitch = 0.0f; 
};
}