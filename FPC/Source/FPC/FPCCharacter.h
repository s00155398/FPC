#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "FPCCharacter.generated.h"
class UParticleSystemComponent;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UCurveFloat;
UENUM(BlueprintType)
enum Weapons
{
	assaultRifle UMETA(DisplayName = "Assault Rifle"),
	pistol UMETA(DisplayName = "Pistol"),
	shotgun UMETA(DisplayName = "ShotGun"),
	rocket UMETA(DisplayName = "Rocket"),
	grenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	sniper UMETA(DisplayName = "Sniper")
};
UCLASS(config=Game)
class AFPCCharacter : public ACharacter
{
	GENERATED_BODY()
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;


public:
	AFPCCharacter();

protected:
	virtual void BeginPlay();

public:

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent* Mesh1P;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent*	Pistol;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent* Assault_Rifle;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent* Shot_Gun;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent* Rocket_Launcher;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent* Grenade_Launcher;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent* Sniper_Rifle;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AFPCProjectile_AssaultRifle> AssaultRifle_ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AFPCProjectile_Pistol> Pistol_ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AFPCProjectile_Shotgun> Shotgun_ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AFPCProjectile_Rocket> Rocket_ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AFPCProjectile_Grenade> Grenade_ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AFPCProjectile_Sniper> Sniper_ProjectileClass;

	/** AnimMontages to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* AssaultRifle_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* AssaultRifle_Fire_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* AssaultRifle_ADS_FireAnimation;

	/** AnimMontage to play each time we reload*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* AssaultRifle_ReloadingAnimation;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* AssaultRifle_ReloadingAnimation_Montage;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Pistol_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Pistol_ADS_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Pistol_Fire_FireAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Pistol_ReloadingAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Pistol_ReloadingAnimation_Montage;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Shotgun_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Shotgun_Fire_FireAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Shotgun_ReloadingAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Shotgun_ReloadingAnimation_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Shotgun_ADS_FireAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Rocket_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Rocket_Fire_FireAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Rocket_ReloadingAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Rocket_ADS_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* GrenadeLauncher_Fire_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* SniperRifle_Fire_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* SniperRifle_Reloading_Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FTimerHandle FireTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float AssaultRifle_Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float Pistol_Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float Shotgun_Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float Rocket_Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float GrenadeLauncher_Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float Sniper_Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool IsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool IsADS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool IsFiring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool IsShotgunFiring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
		TEnumAsByte<Weapons> weapon;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float AssaultRifle_Rotation_Pitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float AssaultRifle_Rotation_Yaw;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float AssaultRifle_Rotation_Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float Pistol_Rotation_Pitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float Pistol_Rotation_Yaw;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float Pistol_Rotation_Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float Shotgun_Rotation_Pitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float Shotgun_Rotation_Yaw;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
		float Shotgun_Rotation_Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimBlueprintGeneratedClass* AssaultRifle_AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimBlueprintGeneratedClass* Pistol_AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimBlueprintGeneratedClass* ShotGun_AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimBlueprintGeneratedClass* Rocket_AnimClass;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	void OnBeginFire();

	void OnEndFire();

	void Reload();

	void WeaponSelectOne();

	void WeaponSelectTwo();

	void WeaponSelectThree();

	void WeaponSelectFour();

	void WeaponSelectFive();

	void WeaponSelectSix();

	void Crouch();

	void UnCrouch();

	void AimDownSight();

	void ReleaseAim();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);



protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	UFUNCTION(BlueprintImplementableEvent)
	void AimDownBP();
	UFUNCTION(BlueprintImplementableEvent)
	void ReleaseAimBP();

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION()
	void TimelineProgress(float Value);

};

