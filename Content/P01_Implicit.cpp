#include "pch.h"
#include "P01_Implicit.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
P01_Implicit::P01_Implicit(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_indexCount(0),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void P01_Implicit::CreateWindowSizeDependentResources()
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

	static const XMVECTORF32 eye = { -2.0f, -2.0f, 7.0f, 0.0f };      // dont delete
	static const XMVECTORF32 at = { -0.5f, -1.8f, -0.5f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4(&m_constantBufferData.eye, eye);
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void P01_Implicit::Update(DX::StepTimer const& timer)
{
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

// Renders one frame using the vertex and pixel shaders.
void P01_Implicit::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// context->RSSetState(m_noCullRasterizerState.Get());

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
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
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
		m_pixelShader.Get(),
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

	// Draw the object.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
}

void P01_Implicit::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.

	auto loadPipeline01_VSTask = DX::ReadDataAsync(L"P01_VS.cso");
	auto loadPipeline01_PSTask = DX::ReadDataAsync(L"P01_PS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline01_VSTask = loadPipeline01_VSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
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
				&m_pixelShader
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
	auto execPipelines = (createPipeline01_PSTask && createPipeline01_VSTask).then([this]() {

		// Rasterization
		D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;

		auto device = m_deviceResources->GetD3DDevice();

		device->CreateRasterizerState(&rasterizerDesc,
			m_noCullRasterizerState.GetAddressOf());

		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		device->CreateRasterizerState(&rasterizerDesc,
			m_backCullRasterizerState.GetAddressOf());

		rasterizerDesc.CullMode = D3D11_CULL_FRONT;
		device->CreateRasterizerState(&rasterizerDesc,
			m_frontCullRasterizerState.GetAddressOf());

		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		device->CreateRasterizerState(&rasterizerDesc,
			m_wireframeRasterizerState.GetAddressOf());

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

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
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

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
			)
		);
	});

	// Once the cube is loaded, the object is ready to be rendered.
	execPipelines.then([this]() {
		m_loadingComplete = true;
		});
}

void P01_Implicit::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_inputLayout.Reset();
	m_vertexShader.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}