#include"app.h"

void App::DrawPerFrame() {
	m_RotateAngle += 0.025f;
	m_CBV[m_FrameIndex].pBuffer->World = DirectX::XMMatrixRotationY(m_RotateAngle);
	m_CBV[m_FrameIndex].pBuffer->World *= DirectX::XMMatrixRotationX(m_RotateAngle);
}
