#pragma once

namespace _202219807_ACW_700119_D3D11_UWP_APP
{
	// Constant buffer used to send MVP and other matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct CameraTrackingBuffer
	{
		DirectX::XMFLOAT3 position;
		float padding;
	};

	struct ElapsedTimeBuffer
	{
		float time;
		DirectX::XMFLOAT3 padding;
	};

	struct ScreenResolutionBuffer
	{
		float resolutionY;
		float resolutionX;
		DirectX::XMFLOAT2 padding;
	};

	struct TessellationFactorBuffer
	{
		float tessellationFactor;
		DirectX::XMFLOAT3 padding;
	};

	struct NoiseStrengthBuffer
	{
		float noiseStrength;
		DirectX::XMFLOAT3 padding;
	};

	struct LightBuffer 
	{
		DirectX::XMFLOAT3 color;
		float depth;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPosition
	{
		DirectX::XMFLOAT3 position;
	};

	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionColorNormal
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 normal;
	};
}