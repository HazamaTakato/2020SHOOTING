#pragma once

#include<d3d12.h>
#include<DirectXMath.h>
#include<d3dx12.h>
#include<unordered_map>
#include<wrl.h>


class Model
{
private: // エイリアス
		 // Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	struct VertexPosNormalUv
	{
		XMFLOAT3 pos;	//xyz
		XMFLOAT3 normal;	//法線
		XMFLOAT2 uv;	//uv
	};

	struct ConstBufferDataB1
	{
		XMFLOAT3 ambient;//アンビエント係数
		float pad1;	//パディング
		XMFLOAT3 diffuse;//ディフューズ係数
		float pad2;	//パディング
		XMFLOAT3 specular;//スペキュラー係数
		float alpha;	//アルファ
	};

	//マテリアル
	struct Material
	{
		std::string name;	//マテリアル名
		XMFLOAT3 ambient;	//アンビエント影響度
		XMFLOAT3 diffuse;	//ディフューズ影響度
		XMFLOAT3 specular;	//スペキュラー影響度
		float alpha;	//アルファ
		std::string textureFilename;//テクスチャファイル名
									//コンストラクタ
		Material() {
			ambient = { 0.3f,0.3f,0.3f };
			diffuse = { 0.0f,0.0f,0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};

private:

	static const std::string baseDirectory;

	static ID3D12Device* device;

	static UINT descriptorHandleIncrementSize;

private:
	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	// インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff;
	// テクスチャバッファ
	ComPtr<ID3D12Resource> texBuff;
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuff; 
	// デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeap;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView;
	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView;
	// 頂点データ配列
	std::vector<VertexPosNormalUv>vertices;
	// 頂点インデックス配列
	std::vector<unsigned short>indices;
	//マテリアル
	Material material;

	std::string name;

	std::unordered_map<std::string, Material*>materials;

public:
	static void staticInitialize(ID3D12Device* device);

	static Model* CreateFromOBJ(const std::string& modelname);

public:

	void Initialize(const std::string& modelname);

	void Update();

	void Draw(ID3D12GraphicsCommandList* cmdList);

private:

	void LoadMaterial(const std::string& directoryPath, const std::string& filename);

	void LoadTexture(const std::string& directoryPath, const std::string& filename);

	void CreateDescriptorHeap();
};
