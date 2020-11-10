#pragma once

#include<Windows.h>
#include<wrl.h>
#include<d3d12.h>
#include<DirectXMath.h>

class Sprite
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	struct VertexPosUv
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	struct ConstBufferData
	{
		XMFLOAT4 color;
		XMMATRIX mat;
	};
public:
	static bool StaticInitialize(ID3D12Device* device, int window_width, int window_height);
	static bool LoadTexture(UINT texnumber, const wchar_t*filename);
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);
	static void PostDraw();
	static Sprite* Create(UINT texNumber, XMFLOAT2 position, XMFLOAT4 color = { 1, 1, 1, 1 }, XMFLOAT2 anchorpoint = { 0.0f, 0.0f }, bool isFlipX = false, bool isFlipY = false);
private:
	static const int srvCount = 512;
	static const int vertNum = 4;
	static ID3D12Device* device;
	static UINT descriptorHandleIncrementSize;
	static ID3D12GraphicsCommandList* cmdList;
	static ComPtr<ID3D12RootSignature> rootSignature;
	static ComPtr<ID3D12PipelineState> pipelineState;
	static XMMATRIX matProjection;
	static ComPtr<ID3D12DescriptorHeap> descHeap;
	static ComPtr<ID3D12Resource> texBuff[srvCount];
public:
	Sprite(UINT texNumber, XMFLOAT2 position, XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY);
	bool Initialize();
	void SetRotation(float rotation);
	void SetPosition(XMFLOAT2 position);
	void SetSize(XMFLOAT2 size);
	void SetAnchorPoint(XMFLOAT2 anchorpoint);
	void SetIsFlipX(bool isFlipX);
	void SetIsFlipY(bool isFlipY);
	void SetTextureRect(XMFLOAT2 texBase, XMFLOAT2 texSize);
	void Draw();
	const XMFLOAT2& GetPosition() { return position; }
private:
	ComPtr<ID3D12Resource> vertBuff;
	ComPtr<ID3D12Resource> constBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	UINT texNumber = 0;
	float rotation = 0.0f;
	XMFLOAT2 position{};
	XMFLOAT2 size = { 100.0f, 100.0f };
	XMFLOAT2 anchorpoint = { 0, 0 };
	XMMATRIX matWorld{};
	XMFLOAT4 color = { 1, 1, 1, 1 };
	bool isFlipX = false;
	bool isFlipY = false;
	XMFLOAT2 texBase = { 0, 0 };
	XMFLOAT2 texSize = { 100.0f, 100.0f };
private:
	void TransferVertices();
};
