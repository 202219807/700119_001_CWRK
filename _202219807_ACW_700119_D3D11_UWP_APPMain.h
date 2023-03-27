#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\P01_Implicit.h"
#include "Content\P02_Explicit.h"
#include "Content\P03_Explicit.h"
#include "Content\P05_Explicit.h"
#include "Content\FpsTextRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace _202219807_ACW_700119_D3D11_UWP_APP
{
	class _202219807_ACW_700119_D3D11_UWP_APPMain : public DX::IDeviceNotify
	{
	public:
		_202219807_ACW_700119_D3D11_UWP_APPMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~_202219807_ACW_700119_D3D11_UWP_APPMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>				m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<P01_Implicit>						m_p01_Implicit;
		std::unique_ptr<P02_Explicit>						m_p02_Explicit;
		std::unique_ptr<P03_Explicit>						m_p03_Explicit;
		std::unique_ptr<P05_Explicit>						m_p05_Explicit;
		std::unique_ptr<FpsTextRenderer>					m_fpsTextRenderer;

		// Rendering loop timer.
		DX::StepTimer										m_timer;
	};
}