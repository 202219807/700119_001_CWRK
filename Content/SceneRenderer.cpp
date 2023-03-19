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
	m_indexCount(0),
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

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	// static const XMVECTORF32 eye = { 0.0f, 3.0f, 1.0f, 0.0f };
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	// static const XMVECTORF32 eye = { 0.0f, -0.1f, 3.5f, 0.0f };
	// static const XMVECTORF32 at = { 0.0f, 0.7f, 0.0f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

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
void SceneRenderer::Render(DX::StepTimer const& timer)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc,
		m_RasterizerState.GetAddressOf());

	context->RSSetState(m_RasterizerState.Get());
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

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
		m_indexCount,
		0,
		0
	);

	//rasterizerDesc.CullMode = D3D11_CULL_NONE;
	//m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc,
	//	m_RasterizerState.GetAddressOf());

	//context->RSSetState(m_RasterizerState.Get());

	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//// Attach our vertex shader.
	//context->VSSetShader(
	//	m_vertexShader02.Get(),
	//	nullptr,
	//	0
	//);

	//// Send the constant buffer to the graphics device.
	//context->VSSetConstantBuffers1(
	//	0,
	//	1,
	//	m_constantBuffer.GetAddressOf(),
	//	nullptr,
	//	nullptr
	//);

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

	//// Send the constant buffer to the graphics device.
	//context->PSSetConstantBuffers1(
	//	0,
	//	1,
	//	m_constantBuffer.GetAddressOf(),
	//	nullptr,
	//	nullptr
	//);

	//// Draw the object 2.
	//context->DrawIndexed(
	//	486,
	//	0,
	//	0
	//);

	//rasterizerDesc.CullMode = D3D11_CULL_NONE;
	//m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc,
	//	m_RasterizerState.GetAddressOf());

	//context->RSSetState(m_RasterizerState.Get());

	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	//// Attach our vertex shader.
	//context->VSSetShader(
	//	m_vertexShader03.Get(),
	//	nullptr,
	//	0
	//);

	//// Send the constant buffer to the graphics device.
	//context->VSSetConstantBuffers1(
	//	0,
	//	1,
	//	m_constantBuffer.GetAddressOf(),
	//	nullptr,
	//	nullptr
	//);

	//// Attach our pixel shader.
	//context->PSSetShader(
	//	m_pixelShader03.Get(),
	//	nullptr,
	//	0
	//);

	//// Send the constant buffer to the graphics device.
	//context->PSSetConstantBuffers1(
	//	0,
	//	1,
	//	m_constantBuffer.GetAddressOf(),
	//	nullptr,
	//	nullptr
	//);

	//// Draw the object 3.
	//context->DrawIndexed(
	//	486,
	//	0,
	//	0
	//);

}

void SceneRenderer::CreateDeviceDependentResources()
{
	// Graphic Pipeline 01:
	// 
	// Implicit modeling with signed distance based ray marching
	// Ocean surface and underwater effects.
	// Reflective/Refractive Bubbles.
	// Underwater plantations.
	// Underwater coral object(Mandelbulb derivate)

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

	// Once both shaders are loaded, create the mesh.
	auto execPipeline01 = (createPipeline01_PSTask && createPipeline01_VSTask).then([this]() {

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
	execPipeline01.then([this]() {
		m_loadingComplete = true;
		});

	// Graphic Pipeline 02:
	// 
	// A shoal of colourful coral reef fish created as a particle system.

	//// Load shaders asynchronously.
	//auto loadPipeline02_VSTask = DX::ReadDataAsync(L"P02_VS.cso");
	//auto loadPipeline02_GSTask = DX::ReadDataAsync(L"P02_GS.cso");
	//auto loadPipeline02_PSTask = DX::ReadDataAsync(L"P02_PS.cso");

	//// After the vertex shader file is loaded, create the shader and input layout.
	//auto createPipeline02_VSTask = loadPipeline02_VSTask.then([this](const std::vector<byte>& fileData) {
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateVertexShader(
	//			&fileData[0],
	//			fileData.size(),
	//			nullptr,
	//			&m_vertexShader02
	//		)
	//	);

	//	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	//	{
	//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	};

	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateInputLayout(
	//			vertexDesc,
	//			ARRAYSIZE(vertexDesc),
	//			&fileData[0],
	//			fileData.size(),
	//			&m_inputLayout
	//		)
	//	);
	//	});

	//// After the geometry shader file is loaded, create the shader and constant buffer.
	//auto createPipeline02_GSTask = loadPipeline02_GSTask.then([this](const std::vector<byte>& fileData) {
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateGeometryShader(
	//			&fileData[0],
	//			fileData.size(),
	//			nullptr,
	//			&m_geometryShader02
	//		)
	//	);

	//	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateBuffer(
	//			&constantBufferDesc,
	//			nullptr,
	//			&m_constantBuffer
	//		)
	//	);
	//	});

	//// After the pixel shader file is loaded, create the shader and constant buffer.
	//auto createPipeline02_PSTask = loadPipeline02_PSTask.then([this](const std::vector<byte>& fileData) {
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreatePixelShader(
	//			&fileData[0],
	//			fileData.size(),
	//			nullptr,
	//			&m_pixelShader02
	//		)
	//	);

	//	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateBuffer(
	//			&constantBufferDesc,
	//			nullptr,
	//			&m_constantBuffer
	//		)
	//	);
	//	});

	//// Once both shaders are loaded, create the mesh.
	//auto execPipeline02 = (createPipeline02_PSTask && createPipeline02_GSTask && createPipeline02_VSTask).then([this]() {

	//	int m = 10;
	//	int n = 10;

	//	float w = 10.0f;
	//	float d = 10.0f;

	//	float halfWidth = 0.5f * w;
	//	float halfDepth = 0.5f * d;

	//	const auto nVertices = m * n;
	//	const auto nGridUnit = (m - 1) * (n - 1) * 2;

	//	float dx = w / (n - 1);
	//	float dz = d / (m - 1);

	//	constexpr float FLOAT_MIN = -10.0f;
	//	constexpr float FLOAT_MAX = 10.0f;

	//	VertexPositionColor quadVertices[100] = {};

	//	for (int i = 0; i < m; ++i) {
	//		float z = halfDepth - i * dz;
	//		for (int j = 0; j < n; ++j) {
	//			float x = -halfWidth + j * dx;
	//			// Flat
	//			quadVertices[i * n + j].pos = XMFLOAT3(x, 0.0f, z);

	//			// White blueprint
	//			quadVertices[i * n + j].color = XMFLOAT3(1.0f, 1.0f, 1.0f);

	//		}
	//	};


	//	// Create index buffer for m x n grid
	//	WORD quadIndices[486] = {};
	//	int k = 0;

	//	for (int i = 0; i < m - 1; ++i) {
	//		for (int j = 0; j < n - 1; ++j) {

	//			quadIndices[k] = i * n + j;
	//			quadIndices[k + 1] = i * n + (j + 1);
	//			quadIndices[k + 2] = (i + 1) * n + j;
	//			quadIndices[k + 3] = (i + 1) * n + j;
	//			quadIndices[k + 4] = i * n + (j + 1);
	//			quadIndices[k + 5] = (i + 1) * n + (j + 1);

	//			k += 6;
	//		}
	//	}

	//	
	//	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	//	vertexBufferData.pSysMem = quadVertices;
	//	vertexBufferData.SysMemPitch = 0;
	//	vertexBufferData.SysMemSlicePitch = 0;
	//	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(quadVertices), D3D11_BIND_VERTEX_BUFFER);
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateBuffer(
	//			&vertexBufferDesc,
	//			&vertexBufferData,
	//			&m_vertexBuffer
	//		)
	//	);

	//	m_indexCount = ARRAYSIZE(quadIndices);

	//	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	//	indexBufferData.pSysMem = quadIndices;
	//	indexBufferData.SysMemPitch = 0;
	//	indexBufferData.SysMemSlicePitch = 0;
	//	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(quadIndices), D3D11_BIND_INDEX_BUFFER);
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateBuffer(
	//			&indexBufferDesc,
	//			&indexBufferData,
	//			&m_indexBuffer
	//		)
	//	);
	//	});

	//// Once the cube is loaded, the object is ready to be rendered.
	//execPipeline02.then([this]() {
	//	m_loadingComplete = true;
	//	});

	//// Graphic Pipeline 03:
	//// 
	//// Underwater coral object generated procedurally using a vertex shader..

	//// Load shaders asynchronously.
	//auto loadPipeline03_VSTask = DX::ReadDataAsync(L"P03_VS.cso");
	//auto loadPipeline03_PSTask = DX::ReadDataAsync(L"P03_PS.cso");

	//// After the vertex shader file is loaded, create the shader and input layout.
	//auto createPipeline03_VSTask = loadPipeline03_VSTask.then([this](const std::vector<byte>& fileData) {
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateVertexShader(
	//			&fileData[0],
	//			fileData.size(),
	//			nullptr,
	//			&m_vertexShader03
	//		)
	//	);

	//	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	//	{
	//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	};

	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateInputLayout(
	//			vertexDesc,
	//			ARRAYSIZE(vertexDesc),
	//			&fileData[0],
	//			fileData.size(),
	//			&m_inputLayout
	//		)
	//	);
	//	});

	//// After the pixel shader file is loaded, create the shader and constant buffer.
	//auto createPipeline03_PSTask = loadPipeline03_PSTask.then([this](const std::vector<byte>& fileData) {
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreatePixelShader(
	//			&fileData[0],
	//			fileData.size(),
	//			nullptr,
	//			&m_pixelShader03
	//		)
	//	);

	//	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateBuffer(
	//			&constantBufferDesc,
	//			nullptr,
	//			&m_constantBuffer
	//		)
	//	);
	//	});

	//// Once both shaders are loaded, create the mesh.
	//auto execPipeline03 = (createPipeline03_PSTask && createPipeline03_VSTask).then([this]() {

	//	int m = 10;
	//	int n = 10;

	//	float w = 5.0f;
	//	float d = 5.0f;

	//	float halfWidth = 0.5f * w;
	//	float halfDepth = 0.5f * d;

	//	const auto nVertices = m * n;
	//	const auto nGridUnit = (m - 1) * (n - 1) * 2;

	//	float dx = w / (n - 1);
	//	float dz = d / (m - 1);

	//	constexpr float FLOAT_MIN = -10.0f;
	//	constexpr float FLOAT_MAX = 10.0f;

	//	VertexPositionColor quadVertices[100] = {};

	//	for (int i = 0; i < m; ++i) {
	//		float z = halfDepth - i * dz;
	//		for (int j = 0; j < n; ++j) {
	//			float x = -halfWidth + j * dx;
	//			// Flat
	//			quadVertices[i * n + j].pos = XMFLOAT3(x, 0.0f, z);

	//			// White blueprint
	//			quadVertices[i * n + j].color = XMFLOAT3(1.0f, 1.0f, 1.0f);

	//		}
	//	};


	//	// Create index buffer for m x n grid
	//	WORD quadIndices[486] = {};
	//	int k = 0;

	//	for (int i = 0; i < m - 1; ++i) {
	//		for (int j = 0; j < n - 1; ++j) {

	//			quadIndices[k] = i * n + j;
	//			quadIndices[k + 1] = i * n + (j + 1);
	//			quadIndices[k + 2] = (i + 1) * n + j;
	//			quadIndices[k + 3] = (i + 1) * n + j;
	//			quadIndices[k + 4] = i * n + (j + 1);
	//			quadIndices[k + 5] = (i + 1) * n + (j + 1);

	//			k += 6;
	//		}
	//	}


	//	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	//	vertexBufferData.pSysMem = quadVertices;
	//	vertexBufferData.SysMemPitch = 0;
	//	vertexBufferData.SysMemSlicePitch = 0;
	//	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(quadVertices), D3D11_BIND_VERTEX_BUFFER);
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateBuffer(
	//			&vertexBufferDesc,
	//			&vertexBufferData,
	//			&m_vertexBuffer
	//		)
	//	);

	//	m_indexCount = ARRAYSIZE(quadIndices);

	//	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	//	indexBufferData.pSysMem = quadIndices;
	//	indexBufferData.SysMemPitch = 0;
	//	indexBufferData.SysMemSlicePitch = 0;
	//	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(quadIndices), D3D11_BIND_INDEX_BUFFER);
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateBuffer(
	//			&indexBufferDesc,
	//			&indexBufferData,
	//			&m_indexBuffer
	//		)
	//	);
	//	});

	//// Once the cube is loaded, the object is ready to be rendered.
	//execPipeline03.then([this]() {
	//	m_loadingComplete = true;
	//	});

}

void SceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader01.Reset();
	m_vertexShader02.Reset();
	m_vertexShader03.Reset();
	m_inputLayout.Reset();
	m_pixelShader01.Reset();
	m_pixelShader02.Reset();
	m_pixelShader03.Reset();
	m_geometryShader02.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}