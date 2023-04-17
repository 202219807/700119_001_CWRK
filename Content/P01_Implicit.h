#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "ShaderStructures.h"

namespace _202219807_ACW_700119_D3D11_UWP_APP
{
	// Graphic Pipeline 01:
	// 
	// Implicit modeling with ray marching using signed distance function
	// 
	// Ocean surface and underwater effects.
	// Reflective wobbly bubbles.
	// Underwater plantations.
	// Underwater coral object (Mandelbulb derivate).

	using namespace Windows::System;
	using namespace Windows::UI::Core;

	class P01_Implicit
	{
	public:
		P01_Implicit(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection);
		void SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition);
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		void ProcessInput(DX::StepTimer const& timer);
		bool IsKeyPressed(VirtualKey key);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>		    m_deviceResources;

		// Direct3D resources for primitive geometries.	    
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	    m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_indexBuffer;

		// Shader pointers
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader;

		// Rasterization
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_rasterizerState;

		// Constant buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_mvpBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cameraBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_timeBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_lightBuffer;
		
		// System resources for shaders
		ModelViewProjectionConstantBuffer				m_mvpBufferData;
		CameraTrackingBuffer							m_cameraBufferData;
		ElapsedTimeBuffer								m_timeBufferData;
		LightBuffer										m_lightBufferData;
		uint32											m_indexCount;

		DirectX::XMFLOAT3								m_waterColor;
		float											m_waterDepth;

		// Variables used with the rendering loop.
		bool											m_loadingComplete;
	};
}