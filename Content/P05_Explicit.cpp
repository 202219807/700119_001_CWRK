#include "pch.h"
#include "P05_Explicit.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
P05_Explicit::P05_Explicit(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_indexCount(0),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
}

void P05_Explicit::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadPipeline05_VSTask = DX::ReadDataAsync(L"P05_VS.cso");
	auto loadPipeline05_GSTask = DX::ReadDataAsync(L"P05_GS.cso");
	auto loadPipeline05_PSTask = DX::ReadDataAsync(L"P05_PS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline05_VSTask = loadPipeline05_VSTask.then([this](const std::vector<byte>& fileData) {
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

	// After the geometry shader file is loaded, create the shader and constant buffer.
	auto createPipeline05_GSTask = loadPipeline05_GSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_geometryShader
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
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPipeline05_PSTask = loadPipeline05_PSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
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
		});

	// Once both shaders are loaded, create the mesh.
	auto execPipelines = (createPipeline05_PSTask && createPipeline05_GSTask && createPipeline05_VSTask).then([this]() {

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

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = gridVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(gridVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		m_indexCount = ARRAYSIZE(gridIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = gridIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC IndexBufferDesc(sizeof(gridIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&IndexBufferDesc,
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

// Called once per frame, rotates the cube and calculates the model and view matrices.
void P05_Explicit::Update(DX::StepTimer const& timer)
{
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));

	m_timeBufferData.time = timer.GetTotalSeconds();
}

// Renders one frame using the vertex and pixel shaders.
void P05_Explicit::Render()
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);

	// Detach our hull shader.
	context->HSSetShader(
		nullptr,
		nullptr,
		0
	);

	// Detach our domain shader.
	context->DSSetShader(
		nullptr,
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->GSSetConstantBuffers1(
		0,
		1,
		m_mvpBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSSetConstantBuffers1(
		1,
		1,
		m_cameraBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSSetConstantBuffers1(
		2,
		1,
		m_timeBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our geometry shader.
	context->GSSetShader(
		m_geometryShader.Get(),
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

	context->PSSetConstantBuffers1(
		0,
		1,
		m_cameraBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

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

void P05_Explicit::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_inputLayout.Reset();
	m_vertexShader.Reset();
	m_geometryShader.Reset();
	m_pixelShader.Reset();
	m_mvpBuffer.Reset();
	m_cameraBuffer.Reset();
	m_timeBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void P05_Explicit::SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection)
{
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.view, DirectX::XMMatrixTranspose(view));

	DirectX::XMStoreFloat4x4(&m_mvpBufferData.projection, DirectX::XMMatrixTranspose(projection));
}

void P05_Explicit::SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition)
{
	m_cameraBufferData.position = cameraPosition;
}