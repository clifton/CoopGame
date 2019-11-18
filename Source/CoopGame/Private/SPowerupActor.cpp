#include "SPowerupActor.h"


ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNumberOfTicks = 0;
	TicksApplied = 0;
}

void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerupActor::InternalOnPowerupTick()
{
	TicksApplied++;
	OnPowerupTick();

	if (TicksApplied > TotalNumberOfTicks)
	{
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
		UE_LOG(LogTemp, Warning, TEXT("Powerup expired %s"), *GetName());
	}
}

void ASPowerupActor::ActivatePowerup()
{
	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			TimerHandle_PowerupTick,
			this, &ASPowerupActor::InternalOnPowerupTick,
			PowerupInterval, true);
	}
	else
	{
		InternalOnPowerupTick();
	}
	OnActivated();
	UE_LOG(LogTemp, Warning, TEXT("Powerup activated %s"), *GetName());
}
