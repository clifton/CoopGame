#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"


enum class EWaveState : uint8;

// killer controller, victim actor, weapon actor
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnActorKilled, AController*, KillerController, AActor*, VictimActor, AActor*, WeaponActor);

UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	// bots to spawn per wave
	uint16 NumOfBotsToSpawn;
	uint16 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;

	// hook for bp to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	bool IsAnyPlayerAlive();

	void SpawnBotTimerElapsed();

	void StartWave();

	void EndWave();

	// set timer for next wave
	void PrepareForNextWave();

	void CheckWaveState();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RespawnDeadPlayers();

public:

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;

	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
};
