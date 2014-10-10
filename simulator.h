#ifndef __INCLUDE_GUARD_UUID_4325BD40_4D1B_11E4_916C_0800200C9A66__
#define __INCLUDE_GUARD_UUID_4325BD40_4D1B_11E4_916C_0800200C9A66__

#include <cstdint>
#include "settings.h"

class TenhoSimulatorSettings;
class TenhoSimulatorProfile;

struct TenhoSimulatorResult {
	double percentage[GRADE_MAX];
};

struct TenhoSimulatorWorker {
	int32_t point;
	int32_t grade;
	int32_t count[GRADE_MAX];
	
	double sum_rate12;
	double sum_rate123;
};

struct TenhoSimulatorParams {
	int32_t loop_count;
	int32_t set_count;
	int32_t length;
};

class TenhoSimulator {
public:
	TenhoSimulator(
		TenhoSimulatorSettings* settings,
		TenhoSimulatorProfile*  profile
	);
	
	TenhoSimulatorParams param;
	TenhoSimulatorResult compute();

private:
	TenhoSimulatorSettings* settings;
	TenhoSimulatorProfile*  profile;
	
	// 試合を行い順位を返す
	int32_t play(TenhoSimulatorWorker* worker, double rnd);
	
	// 得点を求める
	int32_t getPoint(TenhoSimulatorWorker* worker, int32_t rank);
	
	// 段位の変動を反映する
	void computeGrade(TenhoSimulatorWorker* worker, int32_t point);
};

#endif
