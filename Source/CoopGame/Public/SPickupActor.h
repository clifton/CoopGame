#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"


class USphereComponent;
class UDecalComponent;

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

	virtual void BeginPlay() override;

public:

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
