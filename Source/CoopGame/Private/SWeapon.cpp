// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
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

	FHitResult HitResult;
	bool bIsBlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, ECC_Visibility, QueryParams);
	if (bIsBlockingHit)
	{
		AActor* HitActor = HitResult.GetActor();
		UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, HitResult, MyOwner->GetInstigatorController(), this, DamageType);

		if (ImpactEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
		}

		TracerEndPoint = HitResult.ImpactPoint;
	}

	// DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

	if (TracerEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter("Target", TracerEndPoint);
		}
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
