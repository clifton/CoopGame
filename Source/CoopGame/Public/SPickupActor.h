#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"


class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	

	ASPickupActor();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	TSubclassOf<ASPowerupActor> PowerupClass;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	float CooldownDuration;

	ASPowerupActor* PowerupInstance;

	FTimerHandle TimerHandle_Cooldown;

	virtual void BeginPlay() override;

	UFUNCTION()
	void Respawn();

public:

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
