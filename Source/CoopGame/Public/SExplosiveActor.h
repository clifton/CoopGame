#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveActor.generated.h"


class USHealthComponent;
class URadialForceComponent;
class UParticleSystem;


UCLASS()
class COOPGAME_API ASExplosiveActor : public AActor
{
	GENERATED_BODY()
	
public:	

	ASExplosiveActor();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bExploded, Category = "Explosion")
	bool bExploded;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UMaterialInterface* ExplodedMaterial;

	UFUNCTION()
	void OnDeath(
		USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnRep_bExploded();

	void PlayExplosionEffects();

	virtual void BeginPlay() override;

};
