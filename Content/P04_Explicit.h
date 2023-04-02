#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "ShaderStructures.h"

namespace _202219807_ACW_700119_D3D11_UWP_APP
{
	// Graphic Pipeline 04:
	// 
	// Coral object created by transforming a simple triangle mesh 
	// using a geometry shader.

	class P04_Explicit
	{
	public:
		P04_Explicit(const std::shared_ptr<DX::DeviceResources>& deviceResources);
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
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>	    m_geometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader;

		// Rasterization
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_rasterizerState;

		// Constant buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_mvpBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cameraBuffer;

		// System resources for shaders
		ModelViewProjectionConstantBuffer				m_mvpBufferData;
		CameraPositionConstantBuffer					m_cameraBufferData;
		uint32											m_indexCount;

		// Variables used with the rendering loop.
		bool											m_loadingComplete;
	};
}

