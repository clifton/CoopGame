#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Components/SHealthComponent.h"
#include "CoopGame.h"
#include "SCharacter.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"


ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->DefaultHealth = 50.0f;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::OnHealthChanged);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetCollisionObjectType(COLLISION_TRACKERBOT);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bStartedSelfDestruction = false;
	bExploded = false;

	bUseVelocityChange = true;
	MovementForce = 600.0f;
	RequiredDistanceToTarget = 100.0f;

	PowerLevel = 0;
	MaxPowerLevel = 4;

	RadialDamage = 100.0f;
	DamageRadius = 400.0f;

	SelfDestructTickDamage = 10.0f;
	SelfDamageInterval = 0.25f;

	SetReplicates(true);
}

void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	SphereComp->SetSphereRadius(DamageRadius);

	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();

		GetWorldTimerManager().SetTimer(TimerHandle_UpdatePowerLevel, this, &ASTrackerBot::SetPowerLevel, 0.25f, true, 0.0f);
	}

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	AActor* ClosestActor = nullptr;

	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCharacter::StaticClass(), Characters);
	for (auto* CharPawn : Characters)
	{
		USHealthComponent* CharHealthComp = Cast<USHealthComponent>(CharPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp == nullptr || CharHealthComp->GetHealth() <= 0.0f) continue;

		if (ClosestActor == nullptr || GetDistanceTo(CharPawn) < GetDistanceTo(ClosestActor))
		{
			ClosestActor = CharPawn;
		}
	}

	// use deprecated navigation system
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), ClosestActor);

	// return next path point
	if (NavPath && NavPath->PathPoints.Num() > 1)
	{
		return NavPath->PathPoints[1];
	}

	// failed to find path
	return GetActorLocation();
}

void ASTrackerBot::OnHealthChanged(
	USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float DamageWithPowerLevel = RadialDamage + RadialDamage * (float)PowerLevel;

		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(), DamageWithPowerLevel, DamageWithPowerLevel * 0.2f, GetActorLocation(),
			DamageRadius * 0.2f, DamageRadius, 1.0f, nullptr,
			IgnoredActors, this, GetInstigatorController(), COLLISION_WEAPON);

		DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 12, FColor::Red, false, 4.0f, 0, 1.0f);

		GetWorldTimerManager().ClearTimer(TimerHandle_SelfDamage);
		GetWorldTimerManager().ClearTimer(TimerHandle_UpdatePowerLevel);
		
		// immediate destroy doesnt let animation play on clients
		SetLifeSpan(2.0f);
	}
}

void ASTrackerBot::SetPowerLevel()
{
	TArray<struct FOverlapResult> OverlapResult;
	
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	CollisionObjectQueryParams.AddObjectTypesToQuery(COLLISION_TRACKERBOT);
	
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(DamageRadius);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		OverlapResult, GetActorLocation(), FQuat::FQuat(), CollisionObjectQueryParams,
		CollisionShape, CollisionQueryParams);

	if (PowerLevel != OverlapResult.Num())
	{
		PowerLevel = OverlapResult.Num();

		if (MatInst)
		{
			MatInst->SetScalarParameterValue("PowerLevelAlpha", FMath::Clamp((float)PowerLevel / (float)MaxPowerLevel, 0.0f, 1.0f));
		}
	}
}

// only runs on clients
void ASTrackerBot::OnRep_PowerLevel()
{
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("PowerLevelAlpha", FMath::Clamp((float)PowerLevel / (float)MaxPowerLevel, 0.0f, 1.0f));
	}
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget > RequiredDistanceToTarget)
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			MeshComp->AddForce(ForceDirection * MovementForce, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection * 200.0f, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}
		else
		{
			NextPathPoint = GetNextPathPoint();
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20.0f, 12, FColor::Yellow, false, 0.0f, 2.0f);
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlapped with %s"), *OtherActor->GetName());

	if (bStartedSelfDestruction) return;

	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
	if (PlayerPawn)
	{
		if (Role == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(
				TimerHandle_SelfDamage,
				[this]() { UGameplayStatics::ApplyDamage(this, SelfDestructTickDamage, this->GetInstigatorController(), this, nullptr); },
				SelfDamageInterval, true, 0.0f);
		}

		// sound will play several seconds, so make sure the sound follows the actor
		UGameplayStatics::SpawnSoundAttached(BeginSelfDestructSound, RootComponent);
		bStartedSelfDestruction = true;
	}
}

void ASTrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASTrackerBot, PowerLevel);
}