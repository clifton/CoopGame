// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "SProjectile.h"
#include "SCharacter.h"


void ASProjectileWeapon::Fire()
{
	ASCharacter* MyCharacter = Cast<ASCharacter>(GetOwner());
	if (MyCharacter == nullptr) return;
	if (ProjectileClass == nullptr) return;

	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ProjectileSpawnParams;
	ProjectileSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	ProjectileSpawnParams.Instigator = MyCharacter;

	// spawn the projectile at the muzzle
	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, MuzzleRotation, ProjectileSpawnParams);

	PlayMuzzleFlash();

// 	FVector EyeLocation;
// 	FRotator EyeRotation;
// 	MyCharacter->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	// FVector ShotDirection = EyeRotation.Vector();

	// straight line
	// FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

	// DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
}
