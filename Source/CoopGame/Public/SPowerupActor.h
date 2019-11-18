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

	int32 TicksApplied;

	FTimerHandle TimerHandle_PowerupTick;

	virtual void BeginPlay() override;

	void InternalOnPowerupTick();

public:

	void ActivatePowerup();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTick();
};
