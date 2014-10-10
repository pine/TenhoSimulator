
#include <random>
#include <algorithm>

#include "profile.h"
#include "simulator.h"

TenhoSimulator::TenhoSimulator(
		TenhoSimulatorSettings* settings,
		TenhoSimulatorProfile*  profile
	)
{
	this->settings = settings;
	this->profile  = profile;
	
	this->param.loop_count = 1000;
	this->param.set_count  = 1;
	this->param.length     = 1;
}

TenhoSimulatorResult TenhoSimulator::compute()
{
	// 乱数初期化 (メルセンヌ・ツイスタ)
	std::random_device rnd;
	std::mt19937 mt(rnd());
	std::uniform_real_distribution<> rnd_double(0.0, 1.0);
	
	// 演算初期化
	TenhoSimulatorWorker worker;
	
	// 順位回数初期化
	memset(worker.count, 0, sizeof(worker.count));
	
	// 計算パラメーター初期化
	worker.sum_rate12  = this->profile->rate_1st + this->profile->rate_2nd;
	worker.sum_rate123 = worker.sum_rate12       + this->profile->rate_3rd;
	
	// シミュレーション
	for (int32_t i = 0; i < this->param.set_count; ++i) {
		// セット毎にポイントと段位は初期化
		worker.point = this->profile->point;
		worker.grade = this->profile->grade;
		
		for (int32_t j = 0; j < this->param.loop_count; ++j) {
			int32_t rank  = this->play(&worker, rnd_double(mt));
			int32_t point = this->getPoint(&worker, rank);
			
			this->computeGrade(&worker, point);
			++worker.count[worker.grade];
		}
	}
	
	// 計算結果
	TenhoSimulatorResult result;
	memset(result.percentage, 0, sizeof(result.percentage));
	
	int32_t all_count = this->param.set_count * this->param.loop_count;
	
	// ゼロ割り対策
	if (all_count == 0) { return result; }
	
	for (int32_t i = 0; i < GRADE_MAX; ++i) {
		result.percentage[i] = (double)worker.count[i] / all_count;
	}
	
	return result;
}

int32_t TenhoSimulator::play(
	TenhoSimulatorWorker* worker,
	double rnd
	)
{
	if (rnd < this->profile->rate_1st) { return 0; }
	if (this->profile->rate_1st <= rnd && rnd < worker->sum_rate12)  { return 1; }
	if (worker->sum_rate12      <= rnd && rnd < worker->sum_rate123) { return 2; }
	
	return 3;
}

int32_t TenhoSimulator::getPoint(
	TenhoSimulatorWorker* worker,
	int32_t               rank
	)
{
	int32_t max_point = INT32_MIN;
	
	for (int32_t i = 0; i < TYPE_MAX; ++i) {
		if (settings->IsCanPlay(i, worker->grade)) {
			int32_t point = this->settings->GetAcquiredPoint(i, worker->grade, rank, this->param.length);
			max_point = std::max(max_point, point);
		}
	}
	
	return max_point;
}

void TenhoSimulator::computeGrade(
	TenhoSimulatorWorker* worker,
	int32_t point
	)
{
	// 昇段ポイント
	int32_t promoted_point = this->settings->GetPromotedPoint(worker->grade);
	
	// 現在のポイントに反映
	worker->point += point;
	
	// 降段 (ゼロは後段しない)
	if (worker->point < 0) {
		// 後段する階級か (後段する場合 true)
		if (this->settings->IsSubsequentStage(worker->grade)) {
			// 初期値
			int32_t initial_point = this->settings->GetInitialPoint(worker->grade - 1);
			
			--worker->grade; // 後段
			worker->point = initial_point;
		}
		
		// 後段しない場合
		else {
			worker->point = 0;
		}
	}
	
	// 昇段
	// 規定ポイント以上 & 鳳凰以外
	else if (promoted_point <= worker->point && worker->grade + 1 < GRADE_MAX) {
		// 初期値
		int32_t initial_point = this->settings->GetInitialPoint(worker->grade + 1);
		
		++worker->grade; // 昇段
		worker->point = initial_point;
	}
}