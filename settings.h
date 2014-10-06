#ifndef __INCLUDE_GUARD_3AD1FBA0_4D06_11E4_916C_0800200C9A66__
#define __INCLUDE_GUARD_3AD1FBA0_4D06_11E4_916C_0800200C9A66__

#include <cstdint>

#define TYPE_MAX    4
#define GRADE_MAX  21
#define RANK_MAX    4
#define LENGTH_MAX  2

class TenhoSimulatorSettings {
public:
	TenhoSimulatorSettings();
	
	// 獲得ポイント
	void SetAcquiredPoint(
		int32_t type,
		int32_t grade,
		int32_t rank,
		int32_t length,
		int32_t point
		);
	
	int32_t GetAcquiredPoint(
		int32_t type,
		int32_t grade,
		int32_t rank,
		int32_t length
		);
	
	// 必要ポイント
	void SetPromotedPoint(
		int32_t grade,
		int32_t point
		);
	
	int32_t GetPromotedPoint(
		int32_t grade
		);
	
	// 後段するかどうか
	void SetSubsequentStage(
		int32_t grade,
		bool    value
		);
	
	bool IsSubsequentStage(
		int32_t grade
		);
	
	// プレイ可能かどうか
	void SetCanPlay(
		int32_t type,
		int32_t grade,
		bool    can_play
		);
	
	bool IsCanPlay(
		int32_t type,
		int32_t grade
		);
	
protected:
	int32_t acquired_point[TYPE_MAX][GRADE_MAX][LENGTH_MAX][RANK_MAX];
	int32_t promoted_point[GRADE_MAX];
	bool    subsequent_stage[GRADE_MAX];
	bool    can_play[TYPE_MAX][GRADE_MAX];

};

#endif
