// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "DrawDebugHelpers.h"

ASProjectileWeapon::ASProjectileWeapon()
{
	bExplodeOnImpact = false;
	FuseSeconds = 1.0f;
}

void ASProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ASProjectileWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr) return;

	PlayMuzzleFlash();

	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();

	// straight line
	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

	DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
}
