#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "SCharacter.h"
#include "Net/UnrealNetwork.h"


// console variables
int32 ASWeapon::DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVar_DebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), ASWeapon::DebugWeaponDrawing,
	TEXT("Draw debug geometry for weapon damage"),
	ECVF_Cheat);

ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
	BulletSpread = 1.0f;
	RateOfFire = 600.0;

	SetReplicates(true);

	// set tick rate
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60.0f / RateOfFire;
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(
		TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::EndFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	// sanity check / anti-cheat
	return true;
}

void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner == nullptr) return;

	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();

	float HalfRad = FMath::DegreesToRadians(BulletSpread);
	ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

	const float WeaponRange = 10000.0f;
	FVector TraceEnd = EyeLocation + (ShotDirection * WeaponRange);

	// smoke trail target parameter, override if hit
	FVector TraceTo = TraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	// trace against mesh instead of simple collision boxes
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	EPhysicalSurface SurfaceType = SurfaceType_Default;

	FHitResult HitResult;
	bool bIsBlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams);
	if (bIsBlockingHit)
	{
		// stop tracer at hit location
		TraceTo = HitResult.ImpactPoint;

		AActor* HitActor = HitResult.GetActor();
		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

		if (Role == ROLE_Authority)
		{	
			ASCharacter* MyCharacter = Cast<ASCharacter>(MyOwner);
			float DamageMultiplier = MyCharacter ? MyCharacter->GetDamageMultiplier() : 1.0f;

			float ActualDamage = BaseDamage * DamageMultiplier;

			if (SurfaceType == SURFACE_FLESHVULERNABLE)
			{
				ActualDamage *= 3.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, MyCharacter->GetController(), this, DamageType);
		}
	}

	LastFiredTime = GetWorld()->TimeSeconds;

	PlayFireEffects(TraceTo);
	PlayImpactEffects(SurfaceType, TraceTo);

	if (Role == ROLE_Authority)
	{
		HitScanTrace.TraceTo = TraceTo;
		HitScanTrace.SurfaceType = SurfaceType;
		// used to force updates
		HitScanTrace.LastFiredTime = LastFiredTime;
	}

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
	if (MyOwner && FireCamShake && MyOwner->IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC) PC->ClientPlayCameraShake(FireCamShake);
	}
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULERNABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	UE_LOG(LogTemp, Warning, TEXT("Update hit scan trace!"));
	// play cosmetic fx
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}