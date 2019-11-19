#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"


UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ASPowerupActor();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNumberOfTicks;

	UPROPERTY(ReplicatedUsing=OnRep_PowerupActivated)
	bool bIsActivated;

	int32 TicksApplied;

	FTimerHandle TimerHandle_PowerupTick;

	void InternalOnPowerupTick();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPowerupStateChange(bool bIsActive);

public:

	void ActivatePowerup(AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated(AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTick();

	UFUNCTION()
	void OnRep_PowerupActivated();
};
