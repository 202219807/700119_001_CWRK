#include "pch.h"
#include "P04_Explicit.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
P04_Explicit::P04_Explicit(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_isWireframe(false),
	m_indexCount(0),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
}

void P04_Explicit::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadPipeline04_VSTask = DX::ReadDataAsync(L"P04_VS.cso");
	auto loadPipeline04_GSTask = DX::ReadDataAsync(L"P04_GS.cso");
	auto loadPipeline04_PSTask = DX::ReadDataAsync(L"P04_PS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline04_VSTask = loadPipeline04_VSTask.then([this](const std::vector<byte>& fileData) {
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
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	auto createPipeline04_GSTask = loadPipeline04_GSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_geometryShader
			)
		);
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPipeline04_PSTask = loadPipeline04_PSTask.then([this](const std::vector<byte>& fileData) {
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

		});

	// Once both shaders are loaded, create the mesh.
	auto execPipelines = (createPipeline04_PSTask && createPipeline04_GSTask && createPipeline04_VSTask).then([this]() {

		// Cube

		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColorNormal cubeVertices[] =
		{
			// Top face
					{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f,1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
					{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f,1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  },
					{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f,1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)   },
					{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f,1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  },

					// Bottom face
					{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.9f,0.2f, 0.5f),XMFLOAT3(0.0f, -1.0f, 0.0f) },
					{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.9f,0.2f, 0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
					{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.9f,0.2f, 0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f)  },
					{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.9f,0.2f, 0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

					// Left face
					{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f,0.8f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)  },
					{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f,0.8f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
					{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f,0.8f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)  },
					{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f,0.8f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)   },

					// Right face
					{ XMFLOAT3(1.0f, -1.0f, 1.0f),  XMFLOAT3(0.0f,0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
					{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f,0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
					{ XMFLOAT3(1.0f, 1.0f, -1.0f),  XMFLOAT3(0.0f,0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
					{ XMFLOAT3(1.0f, 1.0f, 1.0f),   XMFLOAT3(0.0f,0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

					// Front face
					{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.4f,1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
					{ XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT3(0.4f,1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
					{ XMFLOAT3(1.0f, 1.0f, -1.0f),   XMFLOAT3(0.4f,1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
					{ XMFLOAT3(-1.0f, 1.0f, -1.0f),  XMFLOAT3(0.4f,1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

					// Back face
					{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.5f,0.5f, 0.8f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
					{ XMFLOAT3(1.0f, -1.0f, 1.0f),  XMFLOAT3(0.5f,0.5f, 0.8f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
					{ XMFLOAT3(1.0f, 1.0f, 1.0f),   XMFLOAT3(0.5f,0.5f, 0.8f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
					{ XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3(0.5f,0.5f, 0.8f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
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
			3,1,0,
			2,1,3,

			6,4,5,
			7,4,6,

			11,9,8,
			10,9,11,

			14,12,13,
			15,12,14,

			19,17,16,
			18,17,19,

			22,20,21,
			23,20,22
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

// Called once per frame, rotates the cube and calculates the model and view matrices.
void P04_Explicit::Update(DX::StepTimer const& timer)
{
	ProcessInput(timer);
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
}

// Renders one frame using the vertex and pixel shaders.
void P04_Explicit::Render()
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

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColorNormal);
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //TRIANGLELIST

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

	// Attach our geometry shader.
	context->GSSetShader(
		m_geometryShader.Get(),
		nullptr,
		0
	);

	// Rasterization
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

void P04_Explicit::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_inputLayout.Reset();
	m_vertexShader.Reset();
	m_geometryShader.Reset();
	m_pixelShader.Reset();
	m_mvpBuffer.Reset();
	m_cameraBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void P04_Explicit::SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection)
{
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.view, DirectX::XMMatrixTranspose(view));

	DirectX::XMStoreFloat4x4(&m_mvpBufferData.projection, DirectX::XMMatrixTranspose(projection));
}

void P04_Explicit::SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition)
{
	m_cameraBufferData.position = cameraPosition;
}

void P04_Explicit::ProcessInput(DX::StepTimer const& timer)
{
	if (IsKeyPressed(VirtualKey::F4))
	{
		m_isWireframe = !m_isWireframe;

		D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
		rasterizerDesc.CullMode = D3D11_CULL_NONE;

		if (m_isWireframe) rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		else  rasterizerDesc.FillMode = D3D11_FILL_SOLID;

		auto device = m_deviceResources->GetD3DDevice();
		device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
	}
}

bool P04_Explicit::IsKeyPressed(VirtualKey key)
{
	auto keyDownState = CoreVirtualKeyStates::Down;
	auto currentKeyState = CoreWindow::GetForCurrentThread()->GetKeyState(key);

	if ((currentKeyState & keyDownState) == keyDownState) return true;
	return false;
}