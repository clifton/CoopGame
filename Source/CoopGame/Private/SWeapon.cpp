// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"


// console variables
int32 ASWeapon::DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVar_DebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), ASWeapon::DebugWeaponDrawing,
	TEXT("Draw debug geometry for weapon damage"),
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

void ASWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr) return;

	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();

	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

	// smoke trail target parameter, override if hit
	FVector TracerEndPoint = TraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	// trace against mesh instead of simple collision boxes
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	FHitResult HitResult;
	bool bIsBlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams);
	if (bIsBlockingHit)
	{
		// stop tracer at hit location
		TracerEndPoint = HitResult.ImpactPoint;

		AActor* HitActor = HitResult.GetActor();
		EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

		float ActualDamage = BaseDamage;

		UParticleSystem* SelectedEffect;
		switch (SurfaceType)
		{
		case SURFACE_FLESHVULERNABLE:
			// 3x headshot damage
			ActualDamage *= 3.0f;
		case SURFACE_FLESHDEFAULT:
			SelectedEffect = FleshImpactEffect;
			break;
		default:
			SelectedEffect = DefaultImpactEffect;
			break;
		}

		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, MyOwner->GetInstigatorController(), this, DamageType);

		if (SelectedEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
		}
	}

	PlayFireEffects(TracerEndPoint);

	if (DebugWeaponDrawing > 0)
	{
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
	}
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter("Target", TracerEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner && FireCamShake)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC) PC->ClientPlayCameraShake(FireCamShake);
	}
}
