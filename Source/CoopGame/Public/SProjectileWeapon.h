// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

UCLASS()
class COOPGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()

public:

	ASProjectileWeapon();
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bExplodeOnImpact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta=(EditCondition="!bExplodeOnImpact"))
	float FuseSeconds;

	virtual void BeginPlay() override;

	virtual void Fire() override;
};
