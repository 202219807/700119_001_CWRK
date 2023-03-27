#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "ShaderStructures.h"

namespace _202219807_ACW_700119_D3D11_UWP_APP 
{
	// Graphic Pipeline 05:
	// 
	// A shoal of colourful coral reef fish created as a particle system.

	class P05_Explicit
	{
	public:
		P05_Explicit(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
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
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_geometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader;

		// Constant buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_constantBuffer;

		// Rasterization
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_wireframeRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_noCullRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_backCullRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_frontCullRasterizerState;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer				m_constantBufferData;
		uint32											m_indexCount;

		// Variables used with the rendering loop.
		bool											m_loadingComplete;

	};
}

