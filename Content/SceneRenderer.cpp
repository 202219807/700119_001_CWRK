#include "pch.h"
#include "SceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_tessellationFactor(31.0f),
	// m_displacementPower(0.4f),
	m_degreesPerSecond(45),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	// Create device independent resources
	ComPtr<IDWriteTextFormat> textFormat;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			L"Courier New",
			nullptr,
			DWRITE_FONT_WEIGHT_MEDIUM,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			21.0f,
			L"en-US",
			&textFormat
		)
	);

	DX::ThrowIfFailed(
		textFormat.As(&m_textFormat)
	);

	DX::ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
	);

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
	);

	m_camera = std::make_unique<Camera>();
	m_camera->SetPosition(0.0f, -2.5f, -15.5f);
	m_camera->SetRotation(0.0f, 0.0f, 0.0f);

	//m_p01_Implicit = std::unique_ptr<P01_Implicit>(new P01_Implicit(m_deviceResources));
	//m_p02_Explicit = std::unique_ptr<P02_Explicit>(new P02_Explicit(m_deviceResources));
	//m_p03_Explicit = std::unique_ptr<P03_Explicit>(new P03_Explicit(m_deviceResources));
	m_p04_Explicit = std::unique_ptr<P04_Explicit>(new P04_Explicit(m_deviceResources));
	//m_p05_Explicit = std::unique_ptr<P05_Explicit>(new P05_Explicit(m_deviceResources));

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
	);


	CreateWindowSizeDependentResources();
}

void SceneRenderer::CreateDeviceDependentResources()
{
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
	);

	//m_p01_Implicit->CreateDeviceDependentResources();
	//m_p02_Explicit->CreateDeviceDependentResources();
	//m_p03_Explicit->CreateDeviceDependentResources();
	m_p04_Explicit->CreateDeviceDependentResources();
	//m_p05_Explicit->CreateDeviceDependentResources();
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources()
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * DirectX::XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	DirectX::XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	DirectX::XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	DirectX::XMStoreFloat4x4(&m_projectionMatrix, perspectiveMatrix * orientationMatrix);
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update(DX::StepTimer const& timer)
{
	std::wstring tessDispText = L"SV_TessFactor: " + std::to_wstring(m_tessellationFactor) + L"\n\n Press < and > to adjust tessellation factor" + 
		L"\n\n Camera at: " + std::to_wstring(m_camera->GetPosition().x + ',') + std::to_wstring(m_camera->GetPosition().y + ',') + std::to_wstring(m_camera->GetPosition().z);
	// \r\n Displacement Power : " + std::to_wstring(m_displacementPower) + L" ([&] to adjust)";

	Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			tessDispText.c_str(),
			(uint32)tessDispText.length(),
			m_textFormat.Get(),
			1200.0f, // Max width of the input text.
			100.0f, // Max height of the input text.
			&textLayout
		)
	);

	DX::ThrowIfFailed(
		textLayout.As(&m_textLayout)
	);

	DX::ThrowIfFailed(
		m_textLayout->GetMetrics(&m_textMetrics)
	);

	ProcessInput(timer);

	//m_p01_Implicit->Update(timer);
	// m_p02_Explicit->Update(timer);
	// m_p03_Explicit->Update(timer);
	m_p04_Explicit->Update(timer);
	// m_p05_Explicit->Update(timer);

}

// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render()
{
	m_camera->Render();

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity();
	m_camera->GetViewMatrix(viewMatrix);

	/*m_p01_Implicit->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_p01_Implicit->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_p01_Implicit->Render();*/

	//m_p02_Explicit->Render();
	//m_p03_Explicit->Render();

	m_p04_Explicit->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_p04_Explicit->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_p04_Explicit->Render();

	//m_p05_Explicit->Render();

	ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
	Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	// Position on the bottom right corner
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		logicalSize.Width - m_textMetrics.layoutWidth - 10,
		m_textMetrics.height
	);

	context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());

	DX::ThrowIfFailed(
		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
	);

	context->DrawTextLayout(
		D2D1::Point2F(0.f, 0.f),
		m_textLayout.Get(),
		m_whiteBrush.Get()
	);

	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
	}

	context->RestoreDrawingState(m_stateBlock.Get());

}

void SceneRenderer::ReleaseDeviceDependentResources()
{
	m_whiteBrush.Reset();

	//m_p01_Implicit->ReleaseDeviceDependentResources();
	//m_p02_Explicit->ReleaseDeviceDependentResources();
	//m_p03_Explicit->ReleaseDeviceDependentResources();
	m_p04_Explicit->ReleaseDeviceDependentResources();
	//m_p05_Explicit->ReleaseDeviceDependentResources();

}

void SceneRenderer::ProcessInput(DX::StepTimer const& timer)
{
	//CoreVirtualKeyStates::Down

	if (IsKeyPressed(VirtualKey::W))
	{
		m_camera->MoveForward(10.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::S))
	{
		m_camera->MoveBackward(10.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::A))
	{
		m_camera->MoveLeft(10.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::D))
	{
		m_camera->MoveRight(10.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::Shift))
	{
		m_camera->AddPositionY(-10.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::Control))
	{
		m_camera->AddPositionY(10.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::Up))
	{
		m_camera->AddRotationY(50.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::Down))
	{
		m_camera->AddRotationY(-50.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::Left))
	{
		m_camera->AddRotationX(50.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::Right))
	{
		m_camera->AddRotationX(-50.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(static_cast<VirtualKey>(VK_OEM_PERIOD)))
	{
		if (m_tessellationFactor > 1.0f)
		{
			m_tessellationFactor -= 1.0f;
		}
	}

	if (IsKeyPressed(static_cast<VirtualKey>(VK_OEM_COMMA)))
	{
		if (m_tessellationFactor < 64.0f)
		{
			m_tessellationFactor += 1.0f;
		}
	}

	//if (IsKeyPressed(static_cast<VirtualKey>(VK_OEM_4)))
	//{
	//	m_displacementPower -= 0.01f;
	//}

	//if (IsKeyPressed(static_cast<VirtualKey>(VK_OEM_6)))
	//{
	//	m_displacementPower += 0.01f;
	//}
}

bool SceneRenderer::IsKeyPressed(VirtualKey key)
{
	return (CoreWindow::GetForCurrentThread()->GetKeyState(key) & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
}