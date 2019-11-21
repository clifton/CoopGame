#include "SGameMode.h"
#include "Components/SHealthComponent.h"
#include "TimerManager.h"
#include "STrackerBot.h"
#include "SGameState.h"
#include "SPlayerState.h"


ASGameMode::ASGameMode()
{
	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	TimeBetweenWaves = 2.0f;
	WaveCount = 1;

	PrimaryActorTick.bCanEverTick = true;
	// in seconds, 0 will tick every frame
	PrimaryActorTick.TickInterval = 1.0f;
}

bool ASGameMode::IsAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			// assert that a health comp exists
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				return true;
			}
		}
	}
	return false;
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	if (--NumOfBotsToSpawn <= 0) EndWave();

	UE_LOG(LogTemp, Warning, TEXT("Spawning new bot %s remaining"), *FString::FromInt(NumOfBotsToSpawn));
}

void ASGameMode::StartWave()
{
	WaveCount++;
	NumOfBotsToSpawn = 2 * WaveCount;
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);

	RespawnDeadPlayers();
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
		if (TestPawn == nullptr || !TestPawn->IsA(ASTrackerBot::StaticClass())) continue;

		USHealthComponent* HealthComp = CastChecked<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyPawnAlive = true;
			break;
		}
	}

	if (!bIsAnyPawnAlive)
	{
		SetWaveState(EWaveState::WaveComplete);
		PrepareForNextWave();
	}
}

void ASGameMode::GameOver()
{
	EndWave();

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("Game over, players died"));
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ASGameMode::RespawnDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
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
	if (!IsAnyPlayerAlive()) GameOver();
}
