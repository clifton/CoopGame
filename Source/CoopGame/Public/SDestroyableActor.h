#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SDestroyableActor.generated.h"


class USHealthComponent;

UCLASS()
class COOPGAME_API ASDestroyableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASDestroyableActor();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DestroyableActor")
	USHealthComponent* HealthComp;

	UPROPERTY(BlueprintReadOnly, Category = "DestroyableActor")
	bool isDestroyed;

	UPROPERTY(EditDefaultsOnly, Category = "DestroyableActor")
	UParticleSystem* DestroyedFX;

	UFUNCTION()
	void ServerOnDeath(
		USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	void BeginPlay();
};
