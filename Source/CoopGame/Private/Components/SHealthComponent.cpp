// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"


USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.0f;
	bIsDead = false;

	SetIsReplicated(true);
}

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (!MyOwner) return;

		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);

		Health = DefaultHealth;
	}
}

void USHealthComponent::HandleTakeAnyDamage(
	AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead) return;

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);
	// UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (InstigatedBy) UE_LOG(LogTemp, Warning, TEXT("Killer Controller: %s"), *InstigatedBy->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Victim Actor: %s"), *DamagedActor->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Weapon Actor: %s"), *DamageCauser->GetName());

	bIsDead = Health <= 0.0f;
	if (bIsDead)
	{
		OnDeath.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

		ASGameMode* GM = GetWorld()->GetAuthGameMode<ASGameMode>();
		if (GM)
		{
			GM->OnActorKilled.Broadcast(InstigatedBy, DamagedActor, DamageCauser);
		}
	}
}

// fired only on clients
void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = OldHealth - Health;
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);

	if (Health <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
	}
}

void USHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f  || Health <= 0.0) return;

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);
	// put behind cvar
	// UE_LOG(LogTemp, Log, TEXT("Health changed to %s - +%s"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
	DOREPLIFETIME(USHealthComponent, bIsDead);
}