#include "pch.h"
#include "SceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources),
	m_isExplicitMode(true),
	m_isDebugMode(false),
	m_showControls(false)
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
			16.0f,
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

	m_p01_Implicit = std::unique_ptr<P01_Implicit>(new P01_Implicit(m_deviceResources));
	m_p02_Explicit = std::unique_ptr<P02_Explicit>(new P02_Explicit(m_deviceResources));
	m_p03_Explicit = std::unique_ptr<P03_Explicit>(new P03_Explicit(m_deviceResources));
	//m_p04_Explicit = std::unique_ptr<P04_Explicit>(new P04_Explicit(m_deviceResources));
	m_p05_Explicit = std::unique_ptr<P05_Explicit>(new P05_Explicit(m_deviceResources));

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

	m_p01_Implicit->CreateDeviceDependentResources();
	m_p02_Explicit->CreateDeviceDependentResources();
	m_p03_Explicit->CreateDeviceDependentResources();
	//m_p04_Explicit->CreateDeviceDependentResources();
	m_p05_Explicit->CreateDeviceDependentResources();
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
	// Update display text.
	uint32 fps = timer.GetFramesPerSecond();

	std::wstring guiHead1 = L"Simulation Controls: ";

	std::wstring guiHead2 = L"Debug info.:\n\n";

	std::wstring guiContext1 =

		L"\n\nSV_TessFactor: " + std::to_wstring(m_p03_Explicit->GetTessellationFactor()) +
		L"\npress < or > to adjust tessellation factor" +
		L"\n\nNoise Scale: " + std::to_wstring(m_p03_Explicit->GetNoiseStrength()) + 
		L"\npress N or M to adjust noise strength"

		L"\n\n'r': enable wireframe mode" +
		L"\n\n'e': render only explicit geometry" +
		L"\n\n't': toggle theme" +
		L"\n\n'c': show camera position (for. debug purpose)" +
		L"\n\n'h': hide controls\n\n\n\n\n";


	std::wstring guiContext2 =

		(fps > 0) ?

		std::to_wstring(fps) + L" FPS" +
		L"\n\nCamera position:\n[" +
		std::to_wstring(m_camera->GetPosition().x) + L"," +
		std::to_wstring(m_camera->GetPosition().y) + L"," +
		std::to_wstring(m_camera->GetPosition().z) + L"]"
		: L" - FPS";

	std::wstring guiText = L"Press 'h' for help";
	if (m_showControls) guiText = guiHead1 + guiContext1;
	if (m_isDebugMode && m_showControls) guiText = guiHead1 + guiContext1 + guiHead2 + guiContext2;
	
	Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			guiText.c_str(),
			(uint32)guiText.length(),
			m_textFormat.Get(),
			1000.0f, // Max width of the input text.
			100.0f,  // Max height of the input text.
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

	m_p01_Implicit->Update(timer);
	m_p02_Explicit->Update(timer);
	m_p03_Explicit->Update(timer);
	//m_p04_Explicit->Update(timer);
	m_p05_Explicit->Update(timer);

}

// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render()
{
	m_camera->Render();

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity();
	m_camera->GetViewMatrix(viewMatrix);

	if (!m_isExplicitMode)
	{
		m_p01_Implicit->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
		m_p01_Implicit->SetCameraPositionConstantBuffer(m_camera->GetPosition());
		m_p01_Implicit->Render();
	}

	m_p02_Explicit->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_p02_Explicit->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_p02_Explicit->Render();

	m_p03_Explicit->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_p03_Explicit->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_p03_Explicit->Render();

	//m_p04_Explicit->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	//m_p04_Explicit->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	//m_p04_Explicit->Render();

	m_p05_Explicit->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_p05_Explicit->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_p05_Explicit->Render();

	ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
	Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	// Position on the bottom right corner
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		20,// logicalSize.Width - m_textMetrics.layoutWidth - 10,
		20 // m_textMetrics.height - 50
	);

	context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());

	DX::ThrowIfFailed(
		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
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

	m_p01_Implicit->ReleaseDeviceDependentResources();
	m_p02_Explicit->ReleaseDeviceDependentResources();
	m_p03_Explicit->ReleaseDeviceDependentResources();
	//m_p04_Explicit->ReleaseDeviceDependentResources();
	m_p05_Explicit->ReleaseDeviceDependentResources();

}

void SceneRenderer::ProcessInput(DX::StepTimer const& timer)
{
	if (IsKeyPressed(VirtualKey::E))
	{
		m_isExplicitMode = !m_isExplicitMode;
	}

	if (IsKeyPressed(VirtualKey::H))
	{
		m_showControls = !m_showControls;
	}

	if (IsKeyPressed(VirtualKey::C))
	{
		m_isDebugMode = !m_isDebugMode;
	}

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

	if (IsKeyPressed(VirtualKey::Control))
	{
		m_camera->AddPositionY(-10.0f * timer.GetElapsedSeconds());
	}

	if (IsKeyPressed(VirtualKey::Shift))
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
}

bool SceneRenderer::IsKeyPressed(VirtualKey key)
{
	auto keyDownState = CoreVirtualKeyStates::Down;
	auto currentKeyState = CoreWindow::GetForCurrentThread()->GetKeyState(key);

	if ((currentKeyState & keyDownState) == keyDownState) return true;
	return false;

}