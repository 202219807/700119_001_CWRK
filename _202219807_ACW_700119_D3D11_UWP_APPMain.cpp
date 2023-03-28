#include "pch.h"
#include "_202219807_ACW_700119_D3D11_UWP_APPMain.h"
#include "Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
_202219807_ACW_700119_D3D11_UWP_APPMain::_202219807_ACW_700119_D3D11_UWP_APPMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_p01_Implicit = std::unique_ptr<P01_Implicit>(new P01_Implicit(m_deviceResources));
	m_p02_Explicit = std::unique_ptr<P02_Explicit>(new P02_Explicit(m_deviceResources));
	m_p03_Explicit = std::unique_ptr<P03_Explicit>(new P03_Explicit(m_deviceResources));
	m_p05_Explicit = std::unique_ptr<P05_Explicit>(new P05_Explicit(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<FpsTextRenderer>(new FpsTextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	
}

_202219807_ACW_700119_D3D11_UWP_APPMain::~_202219807_ACW_700119_D3D11_UWP_APPMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void _202219807_ACW_700119_D3D11_UWP_APPMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_p01_Implicit->CreateWindowSizeDependentResources();
	m_p02_Explicit->CreateWindowSizeDependentResources();
	m_p03_Explicit->CreateWindowSizeDependentResources();
	m_p05_Explicit->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void _202219807_ACW_700119_D3D11_UWP_APPMain::Update() 
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		m_p01_Implicit->Update(m_timer);
		m_p02_Explicit->Update(m_timer);
		m_p03_Explicit->Update(m_timer);
		m_p05_Explicit->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool _202219807_ACW_700119_D3D11_UWP_APPMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	m_p01_Implicit->Render();
	m_p02_Explicit->Render();
	m_p03_Explicit->Render();
	m_p05_Explicit->Render();
	m_fpsTextRenderer->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void _202219807_ACW_700119_D3D11_UWP_APPMain::OnDeviceLost()
{
	m_p01_Implicit->ReleaseDeviceDependentResources();
	m_p02_Explicit->ReleaseDeviceDependentResources();
	m_p03_Explicit->ReleaseDeviceDependentResources();
	m_p05_Explicit->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void _202219807_ACW_700119_D3D11_UWP_APPMain::OnDeviceRestored()
{
	m_p01_Implicit->CreateDeviceDependentResources();
	m_p02_Explicit->CreateDeviceDependentResources();
	m_p03_Explicit->CreateDeviceDependentResources();
	m_p05_Explicit->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
