// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPCProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AFPCProjectile : public AActor
{
	GENERATED_BODY()

	

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Particle, meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* TracerParticleSystem;

	
public:
	AFPCProjectile();

	/** Sphere collision component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	USphereComponent* CollisionComp;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gaemplay, meta = (AllowPrivateAccess = "true"))
	float ImpactMultiplier = 1000.0f;

	/** called when projectile hits something */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

