#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "ShaderStructures.h"

namespace _202219807_ACW_700119_D3D11_UWP_APP
{
	// Graphic Pipeline 03:
	// 
	// Underwater coral objects generated using 
	// parametric surface designing and
	// tessellation with SM5 hull and domain shaders.

	class P03_Explicit
	{
	public:
		P03_Explicit(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection);
		void SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition);
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>		    m_deviceResources;

		// Direct3D resources for primitive geometries.	    
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	    m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_indexBuffer;

		// Shader pointers
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11HullShader>	    m_hullShader;
		Microsoft::WRL::ComPtr<ID3D11DomainShader>	    m_domainShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader;

		// Rasterization
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_rasterizerState;
		
		// Constant buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_mvpBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cameraBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_timeBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_resolutionBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer				m_mvpBufferData;
		CameraTrackingBuffer							m_cameraBufferData;
		ElapsedTimeBuffer								m_timeBufferData;
		ScreenResolutionBuffer							m_resolutionBufferData;
		uint32											m_indexCount;

		// Variables used with the rendering loop.
		bool											m_loadingComplete;

	};
}

