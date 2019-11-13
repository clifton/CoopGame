// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectile.h"
#include "SWeapon.h"
#include "CoopGame.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


// Sets default values
ASProjectile::ASProjectile()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// TODO: set relative rotation?

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CollisionComp->InitCapsuleSize(2.0f, 5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");

	// players cant walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	MeshComp->SetupAttachment(CollisionComp);
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 600.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	ProjectileLifespan = 1.0f;
	BlastRadius = 300.0f;
	BlastDamage = 100.0f;

// 	SetReplicates(true);
// 	SetReplicateMovement(true);
}

void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(TimerHandle_ProjectileLifespan, this, &ASProjectile::OnExplode, ProjectileLifespan);
}

void ASProjectile::OnExplode()
{
	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation());
	}

	// do damage
	float MinimumDamage = BlastDamage * 0.1f;
	float BlastInnerRadius = BlastRadius * 0.25f;
	float DamageFalloff = 1.0f; // linear

	// grenade visibility channel blocking damage application
	float ZOffset = 50.0f; // FMath::Max(CollisionComp->GetScaledCapsuleRadius(), CollisionComp->GetScaledCapsuleHalfHeight() * 2.0f) * 2.0f;
	FVector BlastLocation = GetActorLocation(); // +FVector(0.0f, 0.0f, ZOffset);

	if (Instigator) {
		// UE_LOG(LogTemp, Warning, TEXT("Instigator: %s"), *AActor::GetDebugName(Instigator));

		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		bool bDidCauseDamage = UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BlastDamage, MinimumDamage, BlastLocation, BlastInnerRadius, BlastRadius, DamageFalloff, DamageType, IgnoreActors, this, Instigator->GetController(), COLLISION_WEAPON);
		if (bDidCauseDamage)
		{
			UE_LOG(LogTemp, Warning, TEXT("Grenade caused damage to actor!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Grenade missed!"));
		}
	}

	if (ASWeapon::DebugWeaponDrawing > 0)
	{
		DrawDebugSphere(GetWorld(), BlastLocation, BlastRadius, 12, FColor::Yellow, false, 10.0f);
		DrawDebugSphere(GetWorld(), BlastLocation, BlastInnerRadius, 12, FColor::Red, false, 10.0f);
	}

	this->Destroy();
}