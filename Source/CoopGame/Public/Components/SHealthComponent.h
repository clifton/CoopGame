// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"


// on health changed event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
	FOnHealthChangedSignature,
	USHealthComponent*, HealthComp, float, Health, float, HealthDelta,
	const class UDamageType*, DamageType,
	class AController*, InstigatedBy, AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FClientOnHealthChangedSignature,
	float, Health);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature ServerOnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature ServerOnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FClientOnHealthChangedSignature ClientOnHealthChanged;

	USHealthComponent();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "HealthComponent")
	bool bIsDead;

	virtual void BeginPlay() override;

	UFUNCTION()
	void ServerHandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// only called on clients
	UFUNCTION()
	void OnRep_Health();
};
