#include "SProjectileWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "SProjectile.h"
#include "SCharacter.h"


void ASProjectileWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

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
	
	LastFiredTime = GetWorld()->TimeSeconds;

	if (Role == ROLE_Authority)
	{
		HitScanTrace.LastFiredTime = LastFiredTime;
	}
}
