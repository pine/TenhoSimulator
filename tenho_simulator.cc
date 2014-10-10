#include <string>
#include <sstream>
#include <cstdint>
#include <pthread.h>

#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/cpp/var_array.h"

#include "ppapi_simple/ps_main.h"
#include "ppapi_simple/ps_instance.h"
#include "ppapi_simple/ps_event.h"

#include "settings.h"
#include "profile.h"
#include "simulator.h"

#ifdef SEL_LDR
#define ppapi_simple_main main
#endif

void PrintKVPacket(
	const pp::Var& key,
	const pp::Var& value
	);

std::string VarDictionaryToString(
	const pp::VarDictionary& value
	);

void ComputeMessageHandler(
	const pp::Var& key,
	const pp::Var& value,
    void* user_data
    );

void PrintComputePacket(
	const pp::Var& key,
	const pp::Var& value
	);

void* Compute(
	void* param
	);

void ConvertResultArray(
	pp::VarDictionary&    converted,
	TenhoSimulatorResult* result
	);

void ProfileMessageHandler(
	const pp::Var& key,
	const pp::Var& value,
    void* user_data
    );

void SetProfile(
	const std::string&     key,
	const pp::Var&         value,
	TenhoSimulatorProfile* profile
	);

void SettingMessageHandler(
	const pp::Var& key,
	const pp::Var& value,
	void* user_data
	);

void SetSetting(
	const std::string&     key,
	const pp::Var&         value,
	TenhoSimulatorSettings* settings
	);

void SetSettingAcquiredPoint(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	);

void SetSettingPromotedPoint(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	);

void SetSettingSubsequentStage(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	);

void SetSettingPermission(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	);

// =============================================================== //

static pthread_mutex_t compute_mutex = PTHREAD_MUTEX_INITIALIZER;

// =============================================================== //

const char* BoolToString(bool value) {
	static const char *TRUE  = "true";
	static const char *FALSE = "false";
	
	return value ? TRUE : FALSE;
}

void PrintKVPacket(
	const pp::Var& key,
	const pp::Var& value
	)
{
	PSInstance *instance = PSInstance::GetInstance();
	
	pp::VarDictionary dict(value);
	std::stringstream packet;
	
	packet << "{ " << key.AsString() << ": ";
	packet << "{ key: " << dict.Get("key").DebugString() << ", ";
	
	if (dict.Get("value").is_dictionary()) {
		pp::VarDictionary dict2(dict.Get("value"));
		packet << "value: " << VarDictionaryToString(dict2) << " }}";
	}
	
	else {
		packet << "value: " << dict.Get("value").DebugString() << " }}";
	}
	
	packet << std::endl;
	
	instance->Trace(packet.str().c_str());
}

std::string VarDictionaryToString(
	const pp::VarDictionary& dict
	)
{
	pp::VarArray keys   = dict.GetKeys();
	uint32_t     length = keys.GetLength();
	
	std::stringstream packet;
	
	packet << "{ ";
	
	for (int i = 0; i < length; ++i) {
		if (i > 0) { packet << ", "; }
		packet << keys.Get(i).AsString() << ": ";
		packet << dict.Get(keys.Get(i)).DebugString();
	}
	
	packet << " }";
	
	return packet.str();
}

// --------------------------------------------------------------- //

void ComputeMessageHandler(
	const pp::Var& key,
	const pp::Var& value,
    void* user_data
    )
{
	PSInstance*     instance = PSInstance::GetInstance();
	TenhoSimulator* simulator=
		reinterpret_cast<TenhoSimulator*>(user_data);
	
	instance->Trace("ComputeMessageHandler\n");
	
	assert(key == "compute");
	assert(value.is_dictionary());
	
	pp::VarDictionary dict(value);
	
	assert(dict.HasKey("loopCount"));
	assert(dict.Get("loopCount").is_int());
	assert(dict.HasKey("setCount"));
	assert(dict.Get("setCount").is_int());
	assert(dict.HasKey("length"));
	assert(dict.Get("length").is_int());
	
	PrintComputePacket(key, value);
	
	// Mutex で同時計算数を制限
	if (pthread_mutex_trylock(&compute_mutex) != 0) {
		return; // 失敗
	}
	
	// 計算係数
	simulator->param.loop_count = dict.Get("loopCount").AsInt();
	simulator->param.set_count  = dict.Get("setCount").AsInt();
	simulator->param.length     = dict.Get("length").AsInt();
	
	pthread_t thread;
	pthread_create(&thread, NULL, Compute, simulator);
}

// 計算パケットを表示
void PrintComputePacket(
	const pp::Var& key,
	const pp::Var& value
	)
{
	PSInstance* instance = PSInstance::GetInstance();
	
	pp::VarDictionary dict(value);
	std::stringstream packet;
	
	packet << "{ " << key.AsString() << ": ";
	packet << "{ loopCount: " << dict.Get("loopCount").AsInt();
	packet << ", setCount: " << dict.Get("setCount").AsInt();
	packet << ", length: " << dict.Get("length").AsInt() << " }}";
	packet << std::endl;
	
	instance->Trace(packet.str().c_str());
}

// 計算
void* Compute(
	void* param
	)
{
	PSInstance* instance = PSInstance::GetInstance();
	
	instance->Trace("Compute\n");
	
	// スレッドをデタッチ
	if (pthread_detach(pthread_self()) != 0) {
		pthread_mutex_unlock(&compute_mutex);
		return NULL;
	}
	
	// シミュレーターを取得
	TenhoSimulator* simulator =
		reinterpret_cast<TenhoSimulator*>(param);
	
	// 計算
	TenhoSimulatorResult result = simulator->compute();
	pp::VarDictionary    converted;
	
	// 変換
	ConvertResultArray(converted, &result);
	
	// 結果を送信
	instance->PostMessage(converted);
	
	// ロックを解除
	pthread_mutex_unlock(&compute_mutex);
	return NULL;
}

void ConvertResultArray(
	pp::VarDictionary&    converted,
	TenhoSimulatorResult* result
	)
{
	pp::VarArray array;
	array.SetLength(GRADE_MAX);
	
	for (int32_t i = 0; i < GRADE_MAX; ++i) {
		array.Set(i, pp::Var(result->percentage[i]));
	}
	
	converted.Set("computed", array);
}

// --------------------------------------------------------------- //

// プロフィールデータのメッセージ
void ProfileMessageHandler(
	const pp::Var& key,
	const pp::Var& value,
    void* user_data
    )
{
	PSInstance*            instance = PSInstance::GetInstance();
	TenhoSimulatorProfile* profile  =
		reinterpret_cast<TenhoSimulatorProfile*>(user_data);
	
	instance->Trace("ProfileMessageHandler\n");
	
	assert(key == "profile");
	assert(value.is_dictionary());
	
	pp::VarDictionary dict(value);
	
	assert(dict.HasKey("key"));
	assert(dict.HasKey("value"));
	assert(dict.Get("key").is_string());
	
	// 設定パケットを表示
	PrintKVPacket(key, value);
	
	std::string profile_key   = dict.Get("key").AsString();
	pp::Var     profile_value = dict.Get("value");
	
	// 設定を適応
	SetProfile(profile_key, profile_value, profile);
}

// プロフィールを適応する
void SetProfile(
	const std::string&     key,
	const pp::Var&         value,
	TenhoSimulatorProfile* profile
	)
{
	if (key == "point") { profile->point = value.AsInt(); }
	if (key == "grade") { profile->grade = value.AsInt(); }
	
	if (key == "rate_1st") { profile->rate_1st = value.AsDouble(); }
	if (key == "rate_2nd") { profile->rate_2nd = value.AsDouble(); }
	if (key == "rate_3rd") { profile->rate_3rd = value.AsDouble(); }
	if (key == "rate_4th") { profile->rate_4th = value.AsDouble(); }
}

// --------------------------------------------------------------- //

void SettingMessageHandler(
	const pp::Var& key,
	const pp::Var& value,
	void* user_data
	)
{
	PSInstance*             instance = PSInstance::GetInstance();
	TenhoSimulatorSettings* settings =
		reinterpret_cast<TenhoSimulatorSettings*>(user_data);
	
	instance->Trace("SettingMessageHandler\n");
	
	assert(key == "setting");
	assert(value.is_dictionary());
	
	pp::VarDictionary dict(value);
	
	assert(dict.HasKey("key"));
	assert(dict.HasKey("value"));
	assert(dict.Get("key").is_string());
	
	// 設定パケットを表示
	PrintKVPacket(key, value);
	
	std::string setting_key   = dict.Get("key").AsString();
	pp::Var     setting_value = dict.Get("value");
	
	// 設定を適応
	SetSetting(setting_key, setting_value, settings);
}

// 設定を適応する
void SetSetting(
	const std::string&     key,
	const pp::Var&         value,
	TenhoSimulatorSettings* settings
	)
{
	if (key == "acquiredPoint") {
		assert(value.is_dictionary());
		
		pp::VarDictionary dict(value);
		SetSettingAcquiredPoint(dict, settings);
	}
	
	if (key == "promotedPoint") {
		assert(value.is_dictionary());
		
		pp::VarDictionary dict(value);
		SetSettingPromotedPoint(dict, settings);
	}
	
	if (key == "isSubsequentStage") {
		assert(value.is_dictionary());
		
		pp::VarDictionary dict(value);
		SetSettingSubsequentStage(dict, settings);
	}
	
	if (key == "permission") {
		assert(value.is_dictionary());
		
		pp::VarDictionary dict(value);
		SetSettingPermission(dict, settings);
	}
}

void SetSettingAcquiredPoint(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	)
{
	assert(value.HasKey("type"));
	assert(value.HasKey("grade"));
	assert(value.HasKey("rank"));
	assert(value.HasKey("length"));
	assert(value.HasKey("point"));
	
	settings->SetAcquiredPoint(
		value.Get("type").AsInt(),
		value.Get("grade").AsInt(),
		value.Get("rank").AsInt(),
		value.Get("length").AsInt(),
		value.Get("point").AsInt()
		);
}

void SetSettingPromotedPoint(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	)
{
	assert(value.HasKey("grade"));
	assert(value.HasKey("point"));
	
	settings->SetPromotedPoint(
		value.Get("grade").AsInt(),
		value.Get("point").AsInt()
		);
}

void SetSettingSubsequentStage(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	)
{
	assert(value.HasKey("grade"));
	assert(value.HasKey("isSubsequentStage"));
	
	settings->SetSubsequentStage(
		value.Get("grade").AsInt(),
		value.Get("isSubsequentStage").AsBool()
		);
}

void SetSettingPermission(
	const pp::VarDictionary& value,
	TenhoSimulatorSettings*  settings
	)
{
	assert(value.HasKey("grade"));
	assert(value.HasKey("type"));
	assert(value.HasKey("canPlay"));
	
	settings->SetCanPlay(
		value.Get("type").AsInt(),
		value.Get("grade").AsInt(),
		value.Get("canPlay").AsBool()
		);
}

// --------------------------------------------------------------- //

int ppapi_simple_main(int argc, char* argv[]) {
	PSInstance *instance = PSInstance::GetInstance();
	TenhoSimulatorProfile  profile;
	TenhoSimulatorSettings settings;
	TenhoSimulator         simulator(&settings, &profile);
	
	// デバッグレベルの設定
//	instance->SetVerbosity(PSInstance::PSV_TRACE);
	instance->SetVerbosity(PSInstance::PSV_ERROR);
	
	instance->RegisterMessageHandler("compute", &ComputeMessageHandler, &simulator);
	instance->RegisterMessageHandler("profile", &ProfileMessageHandler, &profile);
	instance->RegisterMessageHandler("setting", &SettingMessageHandler, &settings);
	
	while (true) {
		PSEvent* ps_event;
		
		while ((ps_event = instance->TryAcquireEvent()) != NULL) {
			instance->ReleaseEvent(ps_event);
		}
	}
	
	return 0;
}

// =============================================================== //

#ifndef SEL_LDR
PPAPI_SIMPLE_REGISTER_MAIN(ppapi_simple_main)
#endif
