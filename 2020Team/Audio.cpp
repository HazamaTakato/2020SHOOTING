#include "Audio.h"
#include <fstream>
#include <cassert>
#include<xapofx.h>
#include<xaudio2fx.h>
#pragma comment(lib,"xaudio2.lib")
//#pragma comment(lib,"xapofx.lib")

bool Audio::Initialize()
{
	HRESULT result;

	// XAudio�G���W���̃C���X�^���X�𐶐�
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if FAILED(result) {
		assert(0);
		return false;
	}

	// �}�X�^�[�{�C�X�𐶐�
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	if FAILED(result) {
		assert(0);
		return false;
	}
	
	return true;
}

void Audio::PlayWave(const char * filename)
{
	HRESULT result;
	// �t�@�C���X�g���[��
	std::ifstream file;
	// Wave�t�@�C�����J��
	file.open(filename, std::ios_base::binary);
	// �t�@�C���I�[�v�����s���`�F�b�N
	if (file.fail()) {
		assert(0);
	}

	// RIFF�w�b�_�[�̓ǂݍ���
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// �t�@�C����RIFF���`�F�b�N
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	// Format�`�����N�̓ǂݍ���
	FormatChunk format;
	file.read((char*)&format, sizeof(format));

	// Data�`�����N�̓ǂݍ���
	Chunk data;
	file.read((char*)&data, sizeof(data));

	// Data�`�����N�̃f�[�^���i�g�`�f�[�^�j�̓ǂݍ���
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Wave�t�@�C�������
	file.close();

	WAVEFORMATEX wfex{};
	// �g�`�t�H�[�}�b�g�̐ݒ�
	memcpy(&wfex, &format.fmt, sizeof(format.fmt));
	wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;

	// �g�`�t�H�[�}�b�g������SourceVoice�̐���
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &wfex, 0, 2.0f, &voiceCallback);
	if FAILED(result) {
		delete[] pBuffer;
		assert(0);
		return;
	}

	// �Đ�����g�`�f�[�^�̐ݒ�
	buf.pAudioData = (BYTE*)pBuffer;
	buf.pContext = pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = data.size;

	des.InitialState = true;
	des.OutputChannels = 2;
	des.pEffect = pXAPO;

	chain.EffectCount = 1;
	chain.pEffectDescriptors = &des;


	// �g�`�f�[�^�̍Đ�
	result = pSourceVoice->SetEffectChain(&chain);
	
	XAUDIO2FX_REVERB_PARAMETERS Effectinfo;
	Effectinfo.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY;
	Effectinfo.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;
	Effectinfo.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
	Effectinfo.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	Effectinfo.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	Effectinfo.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	Effectinfo.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	Effectinfo.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION;
	Effectinfo.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION;
	Effectinfo.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
	Effectinfo.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;
	Effectinfo.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
	Effectinfo.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;
	Effectinfo.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;
	Effectinfo.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;
	Effectinfo.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;
	Effectinfo.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;
	Effectinfo.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN;
	Effectinfo.DecayTime = XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME;
	Effectinfo.Density = XAUDIO2FX_REVERB_DEFAULT_DENSITY;
	Effectinfo.RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE;
	Effectinfo.WetDryMix = XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX;

	result = pSourceVoice->SetEffectParameters(0, &Effectinfo, sizeof(Effectinfo));
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	if FAILED(result) {
		delete[] pBuffer;
		assert(0);
		return;
	}

	result = pSourceVoice->Start();
	if FAILED(result) {
		delete[] pBuffer;
		assert(0);
		return;
	}
}
