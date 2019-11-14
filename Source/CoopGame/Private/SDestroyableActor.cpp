#include "SDestroyableActor.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"


ASDestroyableActor::ASDestroyableActor()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
	isDestroyed = false;
}

void ASDestroyableActor::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnDeath.AddDynamic(this, &ASDestroyableActor::OnDeath);
}

void ASDestroyableActor::OnDeath(
	USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	isDestroyed = true;

	if (DestroyedFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, DestroyedFX, GetActorLocation());
	}
}

