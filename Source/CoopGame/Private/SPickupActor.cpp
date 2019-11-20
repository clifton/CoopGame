#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerupActor.h"
#include "SCharacter.h"


ASPickupActor::ASPickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(75.0f);

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(RootComponent);
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64.0f, 75.0f, 75.0f);

	CooldownDuration = 10.0f;

	SetReplicates(true);
}

void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		Respawn();
	}
}

void ASPickupActor::Respawn()
{
	if (PowerupClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Powerup class is nullptr in %s. Update your blueprint."), *GetName());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	PowerupInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerupClass, GetTransform(), SpawnParams);
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (Role == ROLE_Authority && PowerupInstance && Cast<ASCharacter>(OtherActor))
	{
		PowerupInstance->ActivatePowerup(OtherActor);
		PowerupInstance = nullptr;

		// set a timer to respawn
		GetWorldTimerManager().SetTimer(TimerHandle_Cooldown, this, &ASPickupActor::Respawn, CooldownDuration);
	}
}
