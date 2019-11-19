#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"


ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNumberOfTicks = 0;
	TicksApplied = 0;

	bIsActivated = false;

	SetReplicates(true);
}

void ASPowerupActor::InternalOnPowerupTick()
{
	TicksApplied++;
	OnPowerupTick();

	if (TicksApplied > TotalNumberOfTicks)
	{
		OnExpired();
		bIsActivated = false;
		OnRep_PowerupActivated();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
		UE_LOG(LogTemp, Warning, TEXT("Powerup expired %s"), *GetName());
	}
}

void ASPowerupActor::ActivatePowerup(AActor* OtherActor)
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
	OnActivated(OtherActor);
	bIsActivated = true;
	// ActivatePowerup() only runs on server, so manually call on server
	OnRep_PowerupActivated();
	UE_LOG(LogTemp, Warning, TEXT("Powerup activated %s"), *GetName());
}

void ASPowerupActor::OnRep_PowerupActivated()
{
	OnPowerupStateChange(bIsActivated);
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsActivated);
}