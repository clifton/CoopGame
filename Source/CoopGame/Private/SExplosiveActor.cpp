#include "SExplosiveActor.h"
#include "Components/SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CoopGame.h"


ASExplosiveActor::ASExplosiveActor()
{
	bExploded = false;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
	HealthComp->OnDeath.AddDynamic(this, &ASExplosiveActor::OnDeath);
	HealthComp->DefaultHealth = 40.0f;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	ExplosionImpulse = 400.0f;
	ExplosionDamage = 150.0f;
	ExplosionRadius = 400.0f;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = ExplosionRadius;
	// just change the velocity of objects instead of applying force
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;
}

void ASExplosiveActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASExplosiveActor::OnDeath(USHealthComponent* ChangedHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// already exploded
	if (bExploded) return;

	bExploded = true;

	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
	}
	
	if (ExplodedMaterial)
	{
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}

	if (ExplosionImpulse > 0.0f)
	{
		FVector BoostImpulse = (FVector(0.3f, 0.3f, 0.0f) * FMath::SRand() + FVector::UpVector) * ExplosionImpulse;
		MeshComp->AddImpulse(BoostImpulse, NAME_None, true);
		RadialForceComp->FireImpulse();
	}
	
	if (ExplosionDamage > 0.0f)
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(), ExplosionDamage, ExplosionDamage * 0.1f, GetActorLocation(),
			ExplosionRadius * 0.2f, ExplosionRadius, 1.0f, NULL,
			IgnoreActors, this, InstigatedBy, COLLISION_WEAPON);
	}
}
