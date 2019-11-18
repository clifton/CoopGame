#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"


class USHealthComponent;
class UMaterialInstanceDynamic;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:

	ASTrackerBot();

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RadialDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float DamageRadius;

	FVector NextPathPoint;

	bool bExploded;

	// dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	virtual void BeginPlay() override;

	UFUNCTION()
	FVector GetNextPathPoint();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	void SelfDestruct();

public:	

	virtual void Tick(float DeltaTime) override;

};
