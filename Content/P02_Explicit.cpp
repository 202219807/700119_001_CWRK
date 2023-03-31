#include "pch.h"
#include "P02_Explicit.h"

#include "..\Common\DirectXHelper.h"

using namespace _202219807_ACW_700119_D3D11_UWP_APP;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
P02_Explicit::P02_Explicit(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_indexCount(0),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void P02_Explicit::CreateWindowSizeDependentResources()
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

	static const XMVECTORF32 eye = { 0.0f, 2.0f, 7.0f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4(&m_constantBufferData.eye, eye);
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void P02_Explicit::Update(DX::StepTimer const& timer)
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
void P02_Explicit::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);

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

	// detach our hull shader.
	context->HSSetShader(
		nullptr,
		nullptr,
		0
	);

	// detach our domain shader.
	context->DSSetShader(
		nullptr,
		nullptr,
		0
	);

	// detach our geometry shader.
	context->GSSetShader(
		nullptr,
		nullptr,
		0
	);

	// Rasterization
	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

	auto device = m_deviceResources->GetD3DDevice();

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
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

void P02_Explicit::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadPipeline02_VSTask = DX::ReadDataAsync(L"P02_VS.cso");
	auto loadPipeline02_PSTask = DX::ReadDataAsync(L"P02_PS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPipeline02_VSTask = loadPipeline02_VSTask.then([this](const std::vector<byte>& fileData) {
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
	auto createPipeline02_PSTask = loadPipeline02_PSTask.then([this](const std::vector<byte>& fileData) {
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
	auto execPipelines = (createPipeline02_PSTask && createPipeline02_VSTask).then([this]() {

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


		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = quadVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(quadVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		m_indexCount = ARRAYSIZE(quadIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = quadIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(quadIndices), D3D11_BIND_INDEX_BUFFER);
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

void P02_Explicit::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_inputLayout.Reset();
	m_vertexShader.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}