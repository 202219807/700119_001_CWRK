#include "pch.h"
#include "SceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount01(0),
	m_indexCount02(0),
	m_indexCount03(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	// Default: Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.

	// static const XMVECTORF32 eye = { 0.0f, -0.1f, 2.5f, 0.0f };      // dont delete

	// static const XMVECTORF32 eye = { 0.0f, -1.5f, 10.0f, 0.0f };     // for p01
	// static const XMVECTORF32 at = { 0.0f, -2.0f, 0.0f, 0.0f };

	static const XMVECTORF32 eye = { 0.0f, -1.0f, 15.0f, 0.0f };     // for p01 + p03
	static const XMVECTORF32 at = { 0.0f, -1.8f, 0.5f, 0.0f };


	// static const XMVECTORF32 eye = { 0.0f, 1.0f, 5.5f, 0.0f };     // for p01 + p02 + p03
	//static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };

	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4(&m_constantBufferData.eye, eye);

}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(0.0); // radians;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMVECTOR time = { static_cast<float>(timer.GetTotalSeconds()), 0.0f, 0.0f, 0.0f };
	XMStoreFloat4(&m_constantBufferData.time, time);


	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	context->RSGetViewports(&numViewports, &viewport);

	int viewportWidth = m_deviceResources->GetOutputSize().Width;
	int viewportHeight = m_deviceResources->GetOutputSize().Height;
	XMVECTOR screenSize = { viewportWidth, viewportHeight, 0.0f };
	XMStoreFloat4(&m_constantBufferData.resolution, screenSize);

}

// Rotate the 3D cube model a set amount of radians.
void SceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void SceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void SceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void SceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;

	auto context = m_deviceResources->GetD3DDeviceContext();





	







	// Render pipeline 01
	context->OMSetDepthStencilState(m_depthStencilStateEnv.Get(), 1);

	context->RSSetState(m_envRasterizerState.Get());

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionColor);
	offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer01.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer01.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader01.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader01.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Draw the object 1.
	context->DrawIndexed(
		m_indexCount01,
		0,
		0
	);







	// Render pipeline 02
	//context->OMSetDepthStencilState(m_depthStencilStateObj.Get(), 1);

	//context->RSSetState(m_noCullRasterizerState.Get());

	//// Prepare the constant buffer to send it to the graphics device.
	//context->UpdateSubresource1(
	//	m_constantBuffer.Get(),
	//	0,
	//	NULL,
	//	&m_constantBufferData,
	//	0,
	//	0,
	//	0
	//);

	//// Each vertex is one instance of the VertexPositionColor struct.
	//stride = sizeof(VertexPositionColor);
	//offset = 0;
	//context->IASetVertexBuffers(
	//	0,
	//	1,
	//	m_vertexBuffer02.GetAddressOf(),
	//	&stride,
	//	&offset
	//);

	//context->IASetIndexBuffer(
	//	m_indexBuffer02.Get(),
	//	DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
	//	0
	//);

	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//context->IASetInputLayout(m_inputLayout02.Get());

	//// Attach our vertex shader.
	//context->VSSetShader(
	//	m_vertexShader02.Get(),
	//	nullptr,
	//	0
	//);

	////// Send the constant buffer to the graphics device.
	////context->VSSetConstantBuffers1(
	////	0,
	////	1,
	////	m_constantBuffer.GetAddressOf(),
	////	nullptr,
	////	nullptr
	////);

	//// Attach our geometry shader.
	//context->GSSetShader(
	//	m_geometryShader02.Get(),
	//	nullptr,
	//	0
	//);

	//// Send the constant buffer to the graphics device.
	//context->GSSetConstantBuffers1(
	//	0,
	//	1,
	//	m_constantBuffer.GetAddressOf(),
	//	nullptr,
	//	nullptr
	//);

	//// Attach our pixel shader.
	//context->PSSetShader(
	//	m_pixelShader02.Get(),
	//	nullptr,
	//	0
	//);

	////// Send the constant buffer to the graphics device.
	////context->PSSetConstantBuffers1(
	////	0,
	////	1,
	////	m_constantBuffer.GetAddressOf(),
	////	nullptr,
	////	nullptr
	////);

	//// Draw the object 2.
	//context->DrawIndexed(
	//	m_indexCount02,
	//	0,
	//	0
	//);


	// Render pipeline 03
	context->OMSetDepthStencilState(m_depthStencilStateObj.Get(), 1);

	context->RSSetState(m_wireframeRasterizerState.Get());

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionColor);
	offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer03.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer03.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader03.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader03.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Draw the object 3.
	context->DrawIndexed(
		m_indexCount03,
		0,
		0
	);

	

}

void SceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.

	// Graphic Pipeline 01:
	// 
	// Implicit modeling with signed distance based ray marching
	// Ocean surface and underwater effects.
	// Reflective/Refractive Bubbles.
	// Underwater plantations.
	// Underwater coral object(Mandelbulb derivate)

	auto loadPipeline01_VSTask = DX::ReadDataAsync(L"P01_VS.cso");
	auto loadPipeline01_PSTask = DX::ReadDataAsync(L"P01_PS.cso");

	// Graphic Pipeline 02:
	// 
	// A shoal of colourful coral reef fish created as a particle system.

	auto loadPipeline02_VSTask = DX::ReadDataAsync(L"P02_VS.cso");
	auto loadPipeline02_GSTask = DX::ReadDataAsync(L"P02_GS.cso");
	auto loadPipeline02_PSTask = DX::ReadDataAsync(L"P02_PS.cso");

	// Graphic Pipeline 03:
	// 
	// Underwater coral object generated procedurally using a vertex shader..

	auto loadPipeline03_VSTask = DX::ReadDataAsync(L"P03_VS.cso");
	auto loadPipeline03_PSTask = DX::ReadDataAsync(L"P03_PS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline01_VSTask = loadPipeline01_VSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader01
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPipeline01_PSTask = loadPipeline01_PSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader01
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
		});

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline02_VSTask = loadPipeline02_VSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader02
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout02
			)
		);
		});

	// After the geometry shader file is loaded, create the shader and constant buffer.
	auto createPipeline02_GSTask = loadPipeline02_GSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_geometryShader02
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPipeline02_PSTask = loadPipeline02_PSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader02
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
		});

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline03_VSTask = loadPipeline03_VSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader03
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};


		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPipeline03_PSTask = loadPipeline03_PSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader03
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
		});

	// Once both shaders are loaded, create the mesh.
	auto execPipelines = (createPipeline03_PSTask && createPipeline03_VSTask &&
		createPipeline02_PSTask && createPipeline02_GSTask && createPipeline02_VSTask &&
		createPipeline01_PSTask && createPipeline01_VSTask).then([this]() {

			// Depth stencil
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			// Depth test parameters
			dsDesc.DepthEnable = false;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

			// Stencil test parameters
			dsDesc.StencilEnable = false;
			dsDesc.StencilReadMask = 0xFF;
			dsDesc.StencilWriteMask = 0xFF;

			auto device = m_deviceResources->GetD3DDevice();
			device->CreateDepthStencilState(&dsDesc, &m_depthStencilStateEnv);

			dsDesc.DepthEnable = true;
			device->CreateDepthStencilState(&dsDesc, &m_depthStencilStateObj);

			// Rasterization
			D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

			rasterizerDesc.CullMode = D3D11_CULL_FRONT;
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.ScissorEnable = false;
			rasterizerDesc.DepthBias = 0;
			rasterizerDesc.DepthBiasClamp = 0.0f;
			rasterizerDesc.DepthClipEnable = false;
			rasterizerDesc.MultisampleEnable = false;
			rasterizerDesc.SlopeScaledDepthBias = 0.0f;

			device->CreateRasterizerState(&rasterizerDesc,
				m_envRasterizerState.GetAddressOf());

			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			device->CreateRasterizerState(&rasterizerDesc,
				m_cullRasterizerState.GetAddressOf());

			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			device->CreateRasterizerState(&rasterizerDesc,
				m_noCullRasterizerState.GetAddressOf());

			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			device->CreateRasterizerState(&rasterizerDesc,
				m_wireframeRasterizerState.GetAddressOf());

			// Primitive Geometry

			// Cube

			// Load mesh vertices. Each vertex has a position and a color.
			static const VertexPositionColor cubeVertices[] =
			{
				{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
				{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
				{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
				{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
				{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
				{XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
				{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
				{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
			};

			D3D11_SUBRESOURCE_DATA cubeVertexBufferData = { 0 };
			cubeVertexBufferData.pSysMem = cubeVertices;
			cubeVertexBufferData.SysMemPitch = 0;
			cubeVertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC cubeVertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&cubeVertexBufferDesc,
					&cubeVertexBufferData,
					&m_vertexBuffer01
				)
			);

			// Load mesh indices. Each trio of indices represents
			// a triangle to be rendered on the screen.
			// For example: 0,2,1 means that the vertices with indexes
			// 0, 2 and 1 from the vertex buffer compose the 
			// first triangle of this mesh.
			static const unsigned short cubeIndices[] =
			{
				0,2,1, // -x
				1,2,3,

				4,5,6, // +x
				5,7,6,

				0,1,5, // -y
				0,5,4,

				2,6,7, // +y
				2,7,3,

				0,4,6, // -z
				0,6,2,

				1,3,7, // +z
				1,7,5,
			};

			m_indexCount01 = ARRAYSIZE(cubeIndices);

			D3D11_SUBRESOURCE_DATA cubeIndexBufferData = { 0 };
			cubeIndexBufferData.pSysMem = cubeIndices;
			cubeIndexBufferData.SysMemPitch = 0;
			cubeIndexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC cubeIndexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&cubeIndexBufferDesc,
					&cubeIndexBufferData,
					&m_indexBuffer01
				)
			);

			// M x N 2D Grid

			int m = 10; 
			int n = 10;
			float w = 10.0f; 
			float d = 10.0f;
			float halfWidth = 0.5f * w;
			float halfDepth = 0.5f * d;
			float dx = w / (n - 1);
			float dz = d / (m - 1);

			constexpr float FLOAT_MIN = -10.0f;
			constexpr float FLOAT_MAX = 10.0f;

			VertexPositionColor gridVertices[100] = {};

			for (int i = 0; i < m; ++i) {
				float z = halfDepth - i * dz;
				for (int j = 0; j < n; ++j) {
					float x = -halfWidth + j * dx;
					// Flat
					gridVertices[i * n + j].pos = XMFLOAT3(x, 0.0f, z);

					// White blueprint
					gridVertices[i * n + j].color = XMFLOAT3(1.0f, 1.0f, 1.0f);
				}
			};


			// Create index buffer for m x n grid
			WORD gridIndices[486] = {};
			int k = 0;

			for (int i = 0; i < m - 1; ++i) {
				for (int j = 0; j < n - 1; ++j) {

					gridIndices[k] = i * n + j;
					gridIndices[k + 1] = i * n + (j + 1);
					gridIndices[k + 2] = (i + 1) * n + j;
					gridIndices[k + 3] = (i + 1) * n + j;
					gridIndices[k + 4] = i * n + (j + 1);
					gridIndices[k + 5] = (i + 1) * n + (j + 1);

					k += 6;
				}
			}


			D3D11_SUBRESOURCE_DATA gridVertexBufferData = { 0 };
			gridVertexBufferData.pSysMem = gridVertices;
			gridVertexBufferData.SysMemPitch = 0;
			gridVertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC gridVertexBufferDesc(sizeof(gridVertices), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&gridVertexBufferDesc,
					&gridVertexBufferData,
					&m_vertexBuffer02
				)
			);

			m_indexCount02 = ARRAYSIZE(gridIndices);

			D3D11_SUBRESOURCE_DATA gridIndexBufferData = { 0 };
			gridIndexBufferData.pSysMem = gridIndices;
			gridIndexBufferData.SysMemPitch = 0;
			gridIndexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC gridIndexBufferDesc(sizeof(gridIndices), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&gridIndexBufferDesc,
					&gridIndexBufferData,
					&m_indexBuffer02
				)
			);

			// Quad [-1, 1] x [-1, 1] 

			const UINT numSamples = 100;

			// Load mesh vertices. Each vertex has a position and a color.
			const UINT vSize = (numSamples - 1) * (numSamples - 1);
			const UINT iSize = numSamples * numSamples * 2;

			static VertexPositionColor quadVertices[vSize];
			static unsigned short quadIndices[iSize];


			float xStep = XM_2PI / (numSamples - 1);
			float yStep = XM_PI / (numSamples - 1);

			UINT vertexFlag = 0;
			UINT indexFlag = 0;
			for (UINT i = 0; i < numSamples - 1; i++)
			{
				float y = i * yStep;
				for (UINT j = 0; j < numSamples - 1; j++)
				{
					if (indexFlag > iSize)
						break;
					float x = j * xStep;
					VertexPositionColor v;
					v.pos.x = x;
					v.pos.y = y;
					v.pos.z = 0;
					v.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
					quadVertices[vertexFlag] = v;

					vertexFlag = vertexFlag + 1;

					unsigned short index0 = i * numSamples + j;
					unsigned short index1 = index0 + 1;
					unsigned short index2 = index0 + numSamples;
					unsigned short index3 = index2 + 1;

					quadIndices[indexFlag] = index0;
					quadIndices[indexFlag + 1] = index2;
					quadIndices[indexFlag + 2] = index1;
					quadIndices[indexFlag + 3] = index1;
					quadIndices[indexFlag + 4] = index2;
					quadIndices[indexFlag + 5] = index3;

					indexFlag = indexFlag + 6;
				}
			}


			D3D11_SUBRESOURCE_DATA quadVertexBufferData = { 0 };
			quadVertexBufferData.pSysMem = quadVertices;
			quadVertexBufferData.SysMemPitch = 0;
			quadVertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC quadVertexBufferDesc(sizeof(quadVertices), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&quadVertexBufferDesc,
					&quadVertexBufferData,
					&m_vertexBuffer03
				)
			);

			m_indexCount03 = ARRAYSIZE(quadIndices);

			D3D11_SUBRESOURCE_DATA quadIndexBufferData = { 0 };
			quadIndexBufferData.pSysMem = quadIndices;
			quadIndexBufferData.SysMemPitch = 0;
			quadIndexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC quadIndexBufferDesc(sizeof(quadIndices), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&quadIndexBufferDesc,
					&quadIndexBufferData,
					&m_indexBuffer03
				)
			);

			});

	// Once the cube is loaded, the object is ready to be rendered.
	execPipelines.then([this]() {
		m_loadingComplete = true;
		});
}

void SceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	
	m_inputLayout.Reset();
	
	m_vertexShader01.Reset();
	m_pixelShader01.Reset();
	
	m_vertexShader02.Reset();
	m_geometryShader02.Reset();
	m_pixelShader02.Reset();
	
	m_vertexShader03.Reset();
	m_pixelShader03.Reset();
	
	m_constantBuffer.Reset();
	
	m_vertexBuffer01.Reset();
	m_vertexBuffer02.Reset();
	m_vertexBuffer03.Reset();

	m_indexBuffer01.Reset();
	m_indexBuffer02.Reset();
	m_indexBuffer03.Reset();
}