#include <cstring>
#include "settings.h"

TenhoSimulatorSettings::TenhoSimulatorSettings()
{
	memset(this->acquired_point,   0, sizeof(this->acquired_point));
	memset(this->promoted_point,   0, sizeof(this->promoted_point));
	memset(this->subsequent_stage, 0, sizeof(this->subsequent_stage));
	memset(this->can_play,         0, sizeof(this->can_play));
}

void TenhoSimulatorSettings::SetAcquiredPoint(
	int32_t type,
	int32_t grade,
	int32_t rank,
	int32_t length,
	int32_t point
	)
{
	this->acquired_point[type][grade][length][rank] = point;
}
	
int32_t TenhoSimulatorSettings::GetAcquiredPoint(
	int32_t type,
	int32_t grade,
	int32_t rank,
	int32_t length
	)
{
	return this->acquired_point[type][grade][length][rank];
}

void TenhoSimulatorSettings::SetPromotedPoint(
	int32_t grade,
	int32_t point
	)
{
	this->promoted_point[grade] = point;
}

int32_t TenhoSimulatorSettings::GetPromotedPoint(
	int32_t grade
	)
{
	return this->promoted_point[grade];
}

void TenhoSimulatorSettings::SetSubsequentStage(
	int32_t grade,
	bool    value
	)
{
	this->subsequent_stage[grade] = value;
}

bool TenhoSimulatorSettings::IsSubsequentStage(
	int32_t grade
	)
{
	return this->subsequent_stage[grade];
}

void TenhoSimulatorSettings::SetCanPlay(
	int32_t type,
	int32_t grade,
	bool    can_play
	)
{
	this->can_play[type][grade] = can_play;
}

bool TenhoSimulatorSettings::IsCanPlay(
	int32_t type,
	int32_t grade
	)
{
	return this->can_play[type][grade];
}
