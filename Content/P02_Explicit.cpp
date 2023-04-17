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

		CD3D11_BUFFER_DESC MVPBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&MVPBufferDesc,
				nullptr,
				&m_mvpBuffer
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
	auto execPipelines = (createPipeline02_PSTask && createPipeline02_VSTask).then([this]() {

		const UINT particles = 100;

		// Load mesh vertices. Each vertex has a position and a color.
		const UINT vSize = (particles - 1) * (particles - 1);
		const UINT iSize = particles * particles * 2;

		static VertexPositionColor vertices[vSize];
		static unsigned short indices[iSize];

		float dx = XM_2PI / (particles - 1);
		float dy = XM_PI / (particles - 1);

		UINT vertexFlag = 0;
		UINT indexFlag = 0;
		for (UINT i = 0; i < particles - 1; i++)
		{
			float y = i * dy;
			for (UINT j = 0; j < particles - 1; j++)
			{
				if (indexFlag > iSize)
					break;
				float x = j * dx;
				VertexPositionColor v;
				v.pos.x = x;
				v.pos.y = y;
				v.pos.z = 0;
				v.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
				vertices[vertexFlag] = v;

				vertexFlag = vertexFlag + 1;

				unsigned short index0 = i * particles + j;
				unsigned short index1 = index0 + 1;
				unsigned short index2 = index0 + particles;
				unsigned short index3 = index2 + 1;

				indices[indexFlag] = index0;
				indices[indexFlag + 1] = index2;
				indices[indexFlag + 2] = index1;
				indices[indexFlag + 3] = index1;
				indices[indexFlag + 4] = index2;
				indices[indexFlag + 5] = index3;

				indexFlag = indexFlag + 6;
			}
		}

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = vertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		m_indexCount = ARRAYSIZE(indices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = indices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
			)
		);
		});

	// Once the particles are loaded, the object is ready to be rendered.
	execPipelines.then([this]() {
		m_loadingComplete = true;
		});
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void P02_Explicit::Update(DX::StepTimer const& timer)
{
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
	m_timeBufferData.time = static_cast<float>(timer.GetTotalSeconds());
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
		m_mvpBuffer.Get(),
		0,
		NULL,
		&m_mvpBufferData,
		0,
		0,
		0
	);

	// Prepare the constant buffer to send it to the graphics device.
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST); //D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ //D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ

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
		m_mvpBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->VSSetConstantBuffers1(
		1,
		1,
		m_timeBuffer.GetAddressOf(),
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
	device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
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

void P02_Explicit::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_inputLayout.Reset();
	m_vertexShader.Reset();
	m_pixelShader.Reset();
	m_mvpBuffer.Reset();
	m_cameraBuffer.Reset();
	m_timeBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void P02_Explicit::SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection)
{
	DirectX::XMStoreFloat4x4(&m_mvpBufferData.view, DirectX::XMMatrixTranspose(view));

	DirectX::XMStoreFloat4x4(&m_mvpBufferData.projection, DirectX::XMMatrixTranspose(projection));
}

void P02_Explicit::SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition)
{
	m_cameraBufferData.position = cameraPosition;
}
