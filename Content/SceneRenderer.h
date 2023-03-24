#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

namespace _202219807_ACW_700119_D3D11_UWP_APP
{
	// This sample renderer instantiates a basic rendering pipeline.
	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


	private:
		void Rotate(float radians);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>		    m_deviceResources;
													    
		// Direct3D resources for primitive geometries.	    
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	    m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	    m_inputLayout02;
		
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_vertexBuffer01;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_vertexBuffer02;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_vertexBuffer03;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_indexBuffer01;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_indexBuffer02;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_indexBuffer03;

		// Shader pointers
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader01;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader01;
		
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader02;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>    m_geometryShader02;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader02;
		
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader03;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader03;

		// Constant buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_constantBuffer;
		
		// Depth stencil
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			m_depthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	m_depthStencilStateEnv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	m_depthStencilStateObj;

		// Rasterization
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_cullRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_noCullRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_wireframeRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_envRasterizerState;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer				m_constantBufferData;


		uint32											m_indexCount01;
		uint32											m_indexCount02;
		uint32											m_indexCount03;

		// Variables used with the rendering loop.
		bool											m_loadingComplete;
		float											m_degreesPerSecond;
		bool											m_tracking;
	};
}
