#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"

#include "ShaderStructures.h"

#include "P01_Implicit.h"
//#include "P02_Explicit.h"
//#include "P03_Explicit.h"
//#include "P04_Explicit.h"
//#include "P05_Explicit.h"

#include "Camera.h"

#include <string>

namespace _202219807_ACW_700119_D3D11_UWP_APP
{
	using namespace Windows::System;
	using namespace Windows::UI::Core;

	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		void ProcessInput(DX::StepTimer const& timer);
		bool IsKeyPressed(VirtualKey key);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>				m_deviceResources;
			
		std::unique_ptr<Camera>								m_camera;

		std::unique_ptr<P01_Implicit>						m_p01_Implicit;
		// std::unique_ptr<P02_Explicit>						m_p02_Explicit;
		// std::unique_ptr<P03_Explicit>						m_p03_Explicit;
		// std::unique_ptr<P04_Explicit>						m_p04_Explicit;
		// std::unique_ptr<P05_Explicit>						m_p05_Explicit;

		// Resources related to text rendering.
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1>		m_stateBlock;
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>			m_textFormat;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>		m_whiteBrush;
		Microsoft::WRL::ComPtr<IDWriteTextLayout3>			m_textLayout;
		DWRITE_TEXT_METRICS									m_textMetrics;

		DirectX::XMFLOAT4X4									m_projectionMatrix;

		// Variables used with the rendering loop.
		float												m_tessellationFactor;
		// float												m_displacementPower;

		float												m_degreesPerSecond;
		bool												m_tracking;
	};
}

