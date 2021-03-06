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

private:
	float DamageMultiplier;

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

	UPROPERTY(ReplicatedUsing=OnRep_bDied, BlueprintReadOnly, Category = "Player")
	bool bDied;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Velocity);

	void MoveRight(float Velocity);

	void BeginCrouch();

	void EndCrouch();

	void BeginZoom();

	void EndZoom();

	void EquipWeapon(TSubclassOf<ASWeapon> WeaponClass);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(TSubclassOf<ASWeapon> WeaponClass);

	UFUNCTION()
	void ServerOnDeath(
		USHealthComponent* ChangedHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnRep_bDied();

public:
	// Sets default values for this character's properties
	ASCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFireWeapon();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void EndFireWeapon();

	UFUNCTION(BlueprintCallable, Category = "CharacterAttributes")
	void SetDamageMultiplier(float NewMultiplier);

	UFUNCTION(BlueprintCallable, Category = "CharacterAttributes")
	float GetDamageMultiplier();
};
