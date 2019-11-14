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

	FVector EyeLocation;
	FRotator EyeRotation;
	MyCharacter->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ProjectileSpawnParams;
	ProjectileSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ProjectileSpawnParams.Instigator = MyCharacter;
	ProjectileSpawnParams.Owner = this;

	// spawn the projectile at the muzzle
	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, ProjectileSpawnParams);

	PlayFireEffects();
	// TODO: override tracer effect with smoke arc

	// repeating myself... should clean this up
	LastFiredTime = GetWorld()->TimeSeconds;
}
