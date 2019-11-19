#include "SGameMode.h"
#include "Components/SHealthComponent.h"
#include "TimerManager.h"


ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	// in seconds, 0 will tick every frame
	PrimaryActorTick.TickInterval = 1.0f;
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	if (--NumOfBotsToSpawn <= 0) EndWave();
}

void ASGameMode::StartWave()
{
	WaveCount++;
	NumOfBotsToSpawn = 2 * WaveCount;
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	PrepareForNextWave();
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}

void ASGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	// wave incomplete
	if (NumOfBotsToSpawn > 0 || bIsPreparingForWave) return;

	bool bIsAnyPawnAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || !TestPawn->IsPlayerControlled()) continue;

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() >= 0.0f)
		{
			bIsAnyPawnAlive = true;
			break;
		}
	}

	if (!bIsAnyPawnAlive)
	{
		PrepareForNextWave();
	}
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
}
