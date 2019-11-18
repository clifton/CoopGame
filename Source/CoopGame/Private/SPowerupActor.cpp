#include "SPowerupActor.h"


ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNumberOfTicks = 0;
}

void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerupActor::InternalOnPowerupTick()
{
	TicksApplied++;
	OnPowerupTick();

	if (TicksApplied >= TotalNumberOfTicks)
	{
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::ActivatePowerup()
{
	if (PowerupInterval)
	{
		GetWorldTimerManager().SetTimer(
			TimerHandle_PowerupTick,
			this, &ASPowerupActor::InternalOnPowerupTick,
			PowerupInterval, true, 0.0f);
	}
	else
	{
		InternalOnPowerupTick();
	}
}
