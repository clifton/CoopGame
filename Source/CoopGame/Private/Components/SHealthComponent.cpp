// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/SHealthComponent.h"


USHealthComponent::USHealthComponent()
{
	Health = 100.0f;
}

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}
