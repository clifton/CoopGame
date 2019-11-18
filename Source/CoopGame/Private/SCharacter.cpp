#include "SCharacter.h"
#include "SWeapon.h"
#include "CoopGame.h"
#include "Components/SHealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"


ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// default zoom fov 45
	ZoomFOV = 45.0f;
	ZoomInterpSpeed = 15.0f;

	WeaponAttachSocketName = "WeaponSocket";

	// capsule component should ignore weapon collision channel, pass through to skeletal mesh
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
	bDied = false;

	// kinda bizarre, nav agent properties usually reserved for AI, but needed to enable jumping/crouching
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	// redundant but helpful to document behavior
	if (Role == ROLE_Authority)
	{
		HealthComp->OnDeath.AddDynamic(this, &ASCharacter::ServerOnDeath);
	}

	DefaultFOV = CameraComp->FieldOfView;
	
	EquipWeapon(PrimaryWeaponClass);
}

void ASCharacter::EquipWeapon(TSubclassOf<ASWeapon> WeaponClass)
{
	if (CurrentWeapon && CurrentWeapon->GetClass() == WeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("tried to equip same weapon"));
		return;
	}

	// run this code on controlling client and server
	if (Role != ROLE_Authority)
	{
		ServerEquipWeapon(WeaponClass);
		return;
	}

	// destroy currently held weapon, if one exists
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}
}

void ASCharacter::ServerEquipWeapon_Implementation(TSubclassOf<ASWeapon> WeaponClass)
{
	EquipWeapon(WeaponClass);
}

bool ASCharacter::ServerEquipWeapon_Validate(TSubclassOf<ASWeapon> WeaponClass)
{
	return true;
}

void ASCharacter::ServerOnDeath(
	USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	bDied = true;
	GetMovementComponent()->StopMovementImmediately();
	EndFireWeapon(); // stop firing weapon (only on server)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(10.0f);
	DetachFromControllerPendingDestroy();

	// TFunction / lambda example
	// 	GetWorldTimerManager().SetTimerForNextTick([this]() {
	// 		DetachFromControllerPendingDestroy();
	// 	});
}

// client specific death stuff
void ASCharacter::OnRep_bDied()
{
	if (bDied)
	{
		EndFireWeapon();
	}
}

// velocity is -1.0 - 1.0
void ASCharacter::MoveForward(float Velocity)
{
	AddMovementInput(GetActorForwardVector() * Velocity);
}

void ASCharacter::MoveRight(float Velocity)
{
	AddMovementInput(GetActorRightVector() * Velocity);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void ASCharacter::StartFireWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::EndFireWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->EndFire();
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	float TargetFOV = bWantsToZoom ? ZoomFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// wasd
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	// mouse look
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &ASCharacter::StartFireWeapon);
	PlayerInputComponent->BindAction("FireWeapon", IE_Released, this, &ASCharacter::EndFireWeapon);

	// from https://qiita.com/suzuki_takashi/items/4ac8d25fe10e3a8b1c4f
	DECLARE_DELEGATE_OneParam(FWeaponSelectDelegate, TSubclassOf<ASWeapon>);
	PlayerInputComponent->BindAction<FWeaponSelectDelegate>("EquipPrimaryWeapon", IE_Pressed, this, &ASCharacter::EquipWeapon, PrimaryWeaponClass);
	PlayerInputComponent->BindAction<FWeaponSelectDelegate>("EquipSecondaryWeapon", IE_Pressed, this, &ASCharacter::EquipWeapon, SecondaryWeaponClass);
}

// return camera location instead of eye location
FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp == nullptr) Super::GetPawnViewLocation();
	
	return CameraComp->GetComponentLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}