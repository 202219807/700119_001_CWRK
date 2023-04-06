#include "pch.h"
#include "P03_Explicit.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
P03_Explicit::P03_Explicit(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_indexCount(0),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void P03_Explicit::Update(DX::StepTimer const& timer)
{
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));

	m_timeBufferData.time = timer.GetTotalSeconds();
}

// Renders one frame using the vertex and pixel shaders.
void P03_Explicit::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_mvpBuffer.Get(),
		0,
		NULL,
		&m_mvpBufferData,
		0,
		0,
		0
	);

	context->UpdateSubresource1(
		m_cameraBuffer.Get(),
		0,
		NULL,
		&m_cameraBufferData,
		0,
		0,
		0
	);

	context->UpdateSubresource1(
		m_timeBuffer.Get(),
		0,
		NULL,
		&m_timeBufferData,
		0,
		0,
		0
	);

	context->UpdateSubresource1(
		m_resolutionBuffer.Get(),
		0,
		NULL,
		&m_resolutionBufferData,
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);

	// Attach our hull shader.
	context->HSSetShader(
		m_hullShader.Get(),
		nullptr,
		0
	);

	// Attach our domain shader.
	context->DSSetShader(
		m_domainShader.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->DSSetConstantBuffers1(
		0,
		1,
		m_mvpBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->DSSetConstantBuffers1(
		1,
		1,
		m_cameraBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->DSSetConstantBuffers1(
		2,
		1,
		m_timeBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Detach our geometry shader.
	context->GSSetShader(
		nullptr,
		nullptr,
		0
	);

	// Rasterization
	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

	auto device = m_deviceResources->GetD3DDevice();

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	device->CreateRasterizerState(&rasterizerDesc,
		m_rasterizerState.GetAddressOf());

	context->RSSetState(m_rasterizerState.Get());

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	// Draw the object.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
}

void P03_Explicit::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadPipeline03_VSTask = DX::ReadDataAsync(L"P03_VS.cso");
	auto loadPipeline03_HSTask = DX::ReadDataAsync(L"P03_HS.cso");
	auto loadPipeline03_DSTask = DX::ReadDataAsync(L"P03_DS.cso");
	auto loadPipeline03_PSTask = DX::ReadDataAsync(L"P03_PS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline03_VSTask = loadPipeline03_VSTask.then([this](const std::vector<byte>& fileData) {
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

	// After the hull shader file is loaded, create the shader and constant buffer.
	auto createPipeline03_HSTask = loadPipeline03_HSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateHullShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_hullShader
			)
		);
		});

	// After the domain shader file is loaded, create the shader and constant buffer.
	auto createPipeline03_DSTask = loadPipeline03_DSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateDomainShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_domainShader
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
				&m_pixelShader
			)
		);


		CD3D11_BUFFER_DESC MVPBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&MVPBufferDesc,
				nullptr,
				&m_mvpBuffer
			)
		);


		CD3D11_BUFFER_DESC CameraBufferDesc(sizeof(CameraTrackingBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&CameraBufferDesc,
				nullptr,
				&m_cameraBuffer
			)
		);


		CD3D11_BUFFER_DESC TimeBufferDesc(sizeof(ElapsedTimeBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&TimeBufferDesc,
				nullptr,
				&m_timeBuffer
			)
		);

		CD3D11_BUFFER_DESC ResolutionBufferDesc(sizeof(ScreenResolutionBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&ResolutionBufferDesc,
				nullptr,
				&m_resolutionBuffer
			)
		);
		});

	// Once both shaders are loaded, create the mesh.
	auto execPipelines = (createPipeline03_PSTask && createPipeline03_DSTask && createPipeline03_HSTask && createPipeline03_VSTask).then([this]() {
		
		// Cube geometry

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

void P03_Explicit::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_inputLayout.Reset();
	m_vertexShader.Reset();
	m_hullShader.Reset();
	m_domainShader.Reset();
	m_pixelShader.Reset();
	m_mvpBuffer.Reset();
	m_cameraBuffer.Reset();
	m_timeBuffer.Reset();
	m_resolutionBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void P03_Explicit::SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection)
{
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.view, DirectX::XMMatrixTranspose(view));

	DirectX::XMStoreFloat4x4(&m_mvpBufferData.projection, DirectX::XMMatrixTranspose(projection));
}

void P03_Explicit::SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition)
{
	m_cameraBufferData.position = cameraPosition;
}