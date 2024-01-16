#include"app.h"

void App::DrawPerFrame() {
	m_RotateAngle += 0.025f;
	m_CBV[m_FrameIndex*2+0].pBuffer->World = DirectX::XMMatrixRotationZ(m_RotateAngle + DirectX::XMConvertToRadians(45.0f));
	m_CBV[m_FrameIndex * 2 + 1].pBuffer->World = DirectX::XMMatrixRotationY(m_RotateAngle) * DirectX::XMMatrixScaling(2.0f, 0.5f, 1.0f);

}
