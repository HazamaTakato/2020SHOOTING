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

	// XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if FAILED(result) {
		assert(0);
		return false;
	}

	// マスターボイスを生成
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
	// ファイルストリーム
	std::ifstream file;
	// Waveファイルを開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗をチェック
	if (file.fail()) {
		assert(0);
	}

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format;
	file.read((char*)&format, sizeof(format));

	// Dataチャンクの読み込み
	Chunk data;
	file.read((char*)&data, sizeof(data));

	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる
	file.close();

	WAVEFORMATEX wfex{};
	// 波形フォーマットの設定
	memcpy(&wfex, &format.fmt, sizeof(format.fmt));
	wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &wfex, 0, 2.0f, &voiceCallback);
	if FAILED(result) {
		delete[] pBuffer;
		assert(0);
		return;
	}

	// 再生する波形データの設定
	buf.pAudioData = (BYTE*)pBuffer;
	buf.pContext = pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = data.size;
	buf.LoopCount = 1;//今流してる音+LoopCount回ループする

	des.InitialState = true;
	des.OutputChannels = 2;
	des.pEffect = pXAPO;

	chain.EffectCount = 1;
	chain.pEffectDescriptors = &des;


	// 波形データの再生
	result = pSourceVoice->SetEffectChain(&chain);
	
	XAUDIO2FX_REVERB_PARAMETERS Effectinfo;
	//XAUDIO2FX_REVERB_PARAMETERSの詳細
	//↓のページから
	//https://docs.microsoft.com/ja-jp/windows/win32/api/xaudio2fx/ns-xaudio2fx-xaudio2fx_reverb_parameters
	Effectinfo.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY;//最初の反射の遅延時間
	Effectinfo.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;//最初の反射のリバーブの遅延
	Effectinfo.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;//左右リア出力の遅延
	Effectinfo.SideDelay = XAUDIO2FX_REVERB_DEFAULT_7POINT1_SIDE_DELAY;//左と右の出力の遅延時間※Win10以降でサポート
	Effectinfo.PositionLeft = 30;//リスナーに対するシミュレートされた空間内の左側の入力位置
	Effectinfo.PositionRight = 30;//↑の右側の入力位置
	Effectinfo.PositionMatrixLeft = 30;//左側のソースからリスナーまでの距離の印象 あまり効果がない可能性有
	Effectinfo.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;//↑の右側版
	Effectinfo.EarlyDiffusion = 4;//個々の壁の残響の特性を制御。硬い平面をシミュレートするには最小値に設定し、拡散面をシミュレートするには最大値に設定します
	Effectinfo.LateDiffusion = 15;//↑と同じ
	Effectinfo.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;//1khzでの減衰時間に対して低周波数の減衰時間を調整
	Effectinfo.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;//↑によって制御されるローパスフィルターのコーナー周波数を設定
	Effectinfo.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;//1khzでの減衰時間に対して高周波数の減衰時間の調整
	Effectinfo.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;//↑によって制御されるハイパスフィルターのコーナー周波数を設定
	Effectinfo.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;//ルームエフェクトのローパスフィルターのコーナー周波数を設定
	Effectinfo.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;//初期反射と後期フィールド残響の両方について、ローパスフィルターの通過帯域強度レベルを設定
	Effectinfo.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;//コーナー周波数での初期反射と後期フィールド残響の両方のローパスフィルターの強度を設定
	Effectinfo.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;//初期反射の強度を調整
	Effectinfo.ReverbGain = 20;//残響の強さを調整
	Effectinfo.DecayTime = INFINITY;//1kHzでの残響減衰時間 0.1～無限秒
	Effectinfo.Density = 100;//後期フィールド残響のモード密度を制御
	Effectinfo.RoomSize = 100;//音響空間の見かけのサイズ
	Effectinfo.WetDryMix = a;//リバーブになる出力のパーセント　基本100
	Effectinfo.DisableLateField = FALSE;//TRUEにすると、レイトフィールドリフレクション計算無効。レイトフィールドリフレクションの計算を無効にすると、CPU時間を大幅に節約

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
