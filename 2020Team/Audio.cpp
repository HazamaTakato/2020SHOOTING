#include "Audio.h"
#include <fstream>
#include <cassert>
#include<xapofx.h>
#include<xaudio2fx.h>
#include<xapo.h>
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

void Audio::PlayWave(const char * filename,float a)
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
	buf.LoopCount = 1;//�������Ă鉹+LoopCount�񃋁[�v����

	des.InitialState = true;
	des.OutputChannels = 2;
	des.pEffect = pXAPO;

	chain.EffectCount = 1;
	chain.pEffectDescriptors = &des;


	// �g�`�f�[�^�̍Đ�
	result = pSourceVoice->SetEffectChain(&chain);
	
	XAUDIO2FX_REVERB_PARAMETERS Effectinfo;
	//XAUDIO2FX_REVERB_PARAMETERS�̏ڍ�
	//���̃y�[�W����
	//https://docs.microsoft.com/ja-jp/windows/win32/api/xaudio2fx/ns-xaudio2fx-xaudio2fx_reverb_parameters
	Effectinfo.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY;//�ŏ��̔��˂̒x������
	Effectinfo.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;//�ŏ��̔��˂̃��o�[�u�̒x��
	Effectinfo.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;//���E���A�o�͂̒x��
	Effectinfo.SideDelay = XAUDIO2FX_REVERB_DEFAULT_7POINT1_SIDE_DELAY;//���ƉE�̏o�͂̒x�����ԁ�Win10�ȍ~�ŃT�|�[�g
	Effectinfo.PositionLeft = 30;//���X�i�[�ɑ΂���V�~�����[�g���ꂽ��ԓ��̍����̓��͈ʒu
	Effectinfo.PositionRight = 30;//���̉E���̓��͈ʒu
	Effectinfo.PositionMatrixLeft = 30;//�����̃\�[�X���烊�X�i�[�܂ł̋����̈�� ���܂���ʂ��Ȃ��\���L
	Effectinfo.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;//���̉E����
	Effectinfo.EarlyDiffusion = 4;//�X�̕ǂ̎c���̓����𐧌�B�d�����ʂ��V�~�����[�g����ɂ͍ŏ��l�ɐݒ肵�A�g�U�ʂ��V�~�����[�g����ɂ͍ő�l�ɐݒ肵�܂�
	Effectinfo.LateDiffusion = 15;//���Ɠ���
	Effectinfo.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;//1khz�ł̌������Ԃɑ΂��Ē���g���̌������Ԃ𒲐�
	Effectinfo.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;//���ɂ���Đ��䂳��郍�[�p�X�t�B���^�[�̃R�[�i�[���g����ݒ�
	Effectinfo.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;//1khz�ł̌������Ԃɑ΂��č����g���̌������Ԃ̒���
	Effectinfo.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;//���ɂ���Đ��䂳���n�C�p�X�t�B���^�[�̃R�[�i�[���g����ݒ�
	Effectinfo.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;//���[���G�t�F�N�g�̃��[�p�X�t�B���^�[�̃R�[�i�[���g����ݒ�
	Effectinfo.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;//�������˂ƌ���t�B�[���h�c���̗����ɂ��āA���[�p�X�t�B���^�[�̒ʉߑш拭�x���x����ݒ�
	Effectinfo.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;//�R�[�i�[���g���ł̏������˂ƌ���t�B�[���h�c���̗����̃��[�p�X�t�B���^�[�̋��x��ݒ�
	Effectinfo.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;//�������˂̋��x�𒲐�
	Effectinfo.ReverbGain = 20;//�c���̋����𒲐�
	Effectinfo.DecayTime = INFINITY;//1kHz�ł̎c���������� 0.1�`�����b
	Effectinfo.Density = 100;//����t�B�[���h�c���̃��[�h���x�𐧌�
	Effectinfo.RoomSize = 100;//������Ԃ̌������̃T�C�Y
	Effectinfo.WetDryMix = a;//���o�[�u�ɂȂ�o�͂̃p�[�Z���g�@��{100
	Effectinfo.DisableLateField = FALSE;//TRUE�ɂ���ƁA���C�g�t�B�[���h���t���N�V�����v�Z�����B���C�g�t�B�[���h���t���N�V�����̌v�Z�𖳌��ɂ���ƁACPU���Ԃ�啝�ɐߖ�

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

	if()
}
