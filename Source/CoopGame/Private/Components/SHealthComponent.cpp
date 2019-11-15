// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/SHealthComponent.h"
#include "Net/UnrealNetwork.h"


USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.0f;

	SetIsReplicated(true);
}

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (!MyOwner) return;

		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::ServerHandleTakeAnyDamage);

		Health = DefaultHealth;
		bIsDead = false;
	}
}

void USHealthComponent::ServerHandleTakeAnyDamage(
	AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f) return;

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);
	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));

	ServerOnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (Health <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		ServerOnDeath.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	}
}

void USHealthComponent::OnRep_Health()
{
	ClientOnHealthChanged.Broadcast(Health);
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
	DOREPLIFETIME(USHealthComponent, bIsDead);
}