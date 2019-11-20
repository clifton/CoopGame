#include "SPlayerState.h"


float ASPlayerState::AddScore(float ScoreDelta)
{
	Score += ScoreDelta;
	return Score;
}
