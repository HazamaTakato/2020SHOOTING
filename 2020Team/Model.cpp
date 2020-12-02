#include"Model.h"
#include<fstream>
#include<sstream>
#include <d3dcompiler.h>
#include<DirectXTex.h>

using namespace std;
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

const std::string Model::baseDirectory = "Resources/";
ID3D12Device* Model::device = nullptr;
UINT Model::descriptorHandleIncrementSize = 0;

void Model::staticInitialize(ID3D12Device * device)
{
	assert(!device);

	Model::device = device;

}

Model * Model::CreateFromOBJ(const std::string& modelname)
{
	//メモリ確保
	Model* instance = new Model;
	instance->Initialize(modelname);

	return instance;
}

void Model::Initialize(const std::string & modelname)
{
	const std::string filename = modelname + ".obj";
	const std::string directoryPath = baseDirectory + modelname + "/";

	//ファイルストリーム
	std::ifstream file;

	file.open(directoryPath + filename);

	if (file.fail()) {
		assert(0);
	}

	HRESULT result = S_FALSE;

	name = modelname;

	int indexCountTex = 0;

	vector<XMFLOAT3>positions;//頂点
	vector<XMFLOAT3>normals;//法線
	vector<XMFLOAT2>texcoords;//テクスチャuv

	string line;
	while (getline(file, line))
	{
		//一行分の文字列をストリームに変換
		std::istringstream line_stream(line);

		//半角スペース区切りで行の先頭文字列を取得
		string key;
		getline(line_stream, key, ' ');

		//マテリアル
		if (key == "mtllib")
		{
			string filename;
			line_stream >> filename;
			LoadMaterial(directoryPath, filename);
		}
		if (key == "v")
		{
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			positions.emplace_back(position);
		}
		if (key == "vt")
		{
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			//V方向反転
			texcoord.y = 1.0f - texcoord.y;
			texcoords.emplace_back(texcoord);
		}
		if (key == "vn")
		{
			XMFLOAT3 normal;
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			normals.emplace_back(normal);
		}
		if (key == "usemtl")
		{
			string materialname;
			line_stream >> materialname;
		}
		if (key == "f")
		{
			int faceIndexCount = 0;
			string index_string;

			while (getline(line_stream, index_string, ' '))
			{
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;

				index_stream >> indexPosition;
				Material* material;
				index_stream.seekg(1, ios_base::cur);//スラッシュを飛ばす
				if (material&&material->textureFilename.size() > 0)
				{
					index_stream >> indexTexcoord;
					index_stream.seekg(1, ios_base::cur);
					index_stream >> indexNormal;

					VertexPosNormalUv vertex{};
					vertex.pos = positions[indexPosition - 1];
					vertex.normal = normals[indexNormal - 1];
					vertex.uv = texcoords[indexTexcoord - 1];
					vertices.emplace_back(vertex);
				}
				else 
				{
					char c;
					index_stream >> c;
					if (c == '/')
					{
						VertexPosNormalUv vertex{};
						vertex.pos = positions[indexPosition - 1];
						vertex.normal = { 0,0,1 };
						vertex.uv = { 0,0 };
						vertices.emplace_back(vertex);
					}
					else
					{
						index_stream.seekg(-1, ios_base::cur);//一文字戻る
						index_stream >> indexTexcoord;
						index_stream.seekg(1, ios_base::cur);
						index_stream >> indexNormal;
						VertexPosNormalUv vertex{};
						vertex.pos = positions[indexPosition - 1];
						vertex.normal = normals[indexNormal - 1];
						vertex.uv = { 0,0 };
						vertices.emplace_back(vertex);
					}
				}
				//インデックスデータの追加
				if (faceIndexCount >= 3)
				{
					indices.emplace_back(indexCountTex - 1);
					indices.emplace_back(indexCountTex);
					indices.emplace_back(indexCountTex - 3);
				}
				else
				{
					indices.emplace_back(indexCountTex);
				}
				indexCountTex++;
				faceIndexCount++;
			}
		}
	}

	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv)*vertices.size());
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short)*indices.size());

	//頂点バッファ生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(result)) {
		assert(0);
		return;
	}

	//インデックスバッファ生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));
	if (FAILED(result)) {
		assert(0);
		return;
	}

	//頂点バッファへのデータ転送
	VertexPosNormalUv* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result))
	{
		std::copy(vertices.begin(), vertices.end(), vertMap);
		vertBuff->Unmap(0, nullptr);
	}

	//インデックスバッファへのデータ転送
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	if (SUCCEEDED(result))
	{
		std::copy(indices.begin(), indices.end(), indexMap);
		indexBuff->Unmap(0, nullptr);
	}

	//頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	//インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
}

void Model::Draw(ID3D12GraphicsCommandList * cmdList)
{

}

void Model::Update() 
{
	HRESULT result = S_FALSE;

	ConstBufferDataB1* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result))
	{
		constMap->ambient = material.ambient;
		constMap->diffuse = material.diffuse;
		constMap->specular = material.specular;
		constMap->alpha = material.alpha;
		constBuff->Unmap(0, nullptr);
	}
}

void Model::LoadMaterial(const std::string & directoryPath, const std::string & filename)
{
	std::ifstream file;
	file.open(directoryPath + filename);
	if (file.fail()) {
		assert(0);
	}

	string line;
	while (getline(file, line))
	{
		std::istringstream line_stream(line);

		string key;
		getline(line_stream, key, ' ');

		if (key[0] == '\t')
		{
			key.erase(key.begin());
		}

		if (key == "newmtl")
		{
			line_stream >> material.name;
		}

		if (key == "Ka")
		{
			line_stream >> material.ambient.x;
			line_stream >> material.ambient.y;
			line_stream >> material.ambient.z;
		}
		if (key == "Kd")
		{
			line_stream >> material.diffuse.x;
			line_stream >> material.diffuse.y;
			line_stream >> material.diffuse.z;
		}
		if (key == "Ks") 
		{
			line_stream >> material.specular.x;
			line_stream >> material.specular.y;
			line_stream >> material.specular.z;
		}
		if (key == "map_Kd")
		{
			line_stream >> material.textureFilename;
			LoadTexture(directoryPath, material.textureFilename);
		}

	}
}

void Model::LoadTexture(const std::string & directoryPath, const std::string & filename)
{
	string filepath = directoryPath + filename;

	//ユニコード文字列
	wchar_t wfilepath[128];
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0,
		filepath.c_str(), -1, wfilepath, _countof(wfilepath));

	HRESULT result = S_FALSE;

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(
		wfilepath, WIC_FLAGS_NONE,
		&metadata, scratchImg);

	if (FAILED(result)) {
		assert(0);
	}

	const Image* img = scratchImg.GetImage(0, 0, 0);

	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff)
		);

	if (FAILED(result)) {
		assert(0);
	}

	result = texBuff->WriteToSubresource(
		0,
		nullptr,
		img->pixels,
		(UINT)img->rowPitch,
		(UINT)img->slicePitch
	);
	if (FAILED(result)) {
		assert(0);
	}

	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	D3D12_RESOURCE_DESC resDesc = texBuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texBuff.Get(),
		&srvDesc,
		cpuDescHandleSRV);

}

void Model::CreateDescriptorHeap()
{
	HRESULT result = S_FALSE;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1;
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));
	if (FAILED(result))
	{
		assert(0);
	}

	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}
