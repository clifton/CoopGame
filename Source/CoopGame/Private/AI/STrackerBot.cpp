#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"


ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCanEverAffectNavigation(false);
}

void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
}

FVector ASTrackerBot::GetNextPathPoint()
{
	// HACK
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	// use deprecated navigation system
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	// return next path point
	if (NavPath->PathPoints.Num() > 1)
	{
		return NavPath->PathPoints[1];
	}

	// failed to find path
	return GetActorLocation();
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
