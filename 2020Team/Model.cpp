#include"Model.h"
#include<fstream>
#include<sstream>

using namespace std;

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


}

void Model::Draw(ID3D12GraphicsCommandList * cmdList)
{
}

void Model::LoadMaterial(const std::string & directoryPath, const std::string & filename)
{
}

void Model::LoadTexture(const std::string & directoryPath, const std::string & filename)
{
}

void Model::CreateDescriptorHeap()
{
}
