// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPCProjectile.h"
#include "FPCProjectile_AssaultRifle.generated.h"

/**
 * 
 */
UCLASS()
class FPC_API AFPCProjectile_AssaultRifle : public AFPCProjectile
{
	GENERATED_BODY()
public:
	AFPCProjectile_AssaultRifle();

	float ImpactMultiplier = 2000.0f;
};
