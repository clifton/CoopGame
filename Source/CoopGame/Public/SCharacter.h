// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	USHealthComponent* HealthComp;

	// aim down sights
	bool bWantsToZoom;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin=10.0,ClampMax=90.0))
	float ZoomFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin=0.1,ClampMax=100))
	float ZoomInterpSpeed;

	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> PrimaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> SecondaryWeaponClass;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bDied;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Velocity);

	void MoveRight(float Velocity);

	void BeginCrouch();

	void EndCrouch();

	void BeginZoom();

	void EndZoom();

	void StartFireWeapon();

	void EndFireWeapon();

	void EquipWeapon(TSubclassOf<ASWeapon> WeaponClass);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(TSubclassOf<ASWeapon> WeaponClass);

	UFUNCTION()
	void OnDeath(
		USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);
};
