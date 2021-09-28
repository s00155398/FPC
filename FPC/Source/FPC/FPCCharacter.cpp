// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPCCharacter.h"
#include "FPCProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/TimelineComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "GameFramework/CharacterMovementComponent.h"
DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFPCCharacter
class UCurveFloat;
AFPCCharacter::AFPCCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	Assault_Rifle = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Assault Rifle"));
	Assault_Rifle->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	Assault_Rifle->bCastDynamicShadow = false;
	Assault_Rifle->CastShadow = false;
	Assault_Rifle->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	Assault_Rifle->SetupAttachment(RootComponent);

	Pistol = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pistol"));
	Pistol->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	Pistol->bCastDynamicShadow = false;
	Pistol->CastShadow = false;
	Pistol->SetupAttachment(Mesh1P, TEXT("Pistol_GripPoint"));
	Pistol->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Location"));
	FP_MuzzleLocation->SetupAttachment(Assault_Rifle);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	Pistol_FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Pistol Muzzle Location"));
	Pistol_FP_MuzzleLocation->SetupAttachment(Pistol);
	Pistol_FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
	AssaultRifleAmmo = 30;
	PistolAmmo = 15;
	IsReloading = false;
	IsFiring = false;
	weapon = Weapons::assaultRifle;
	Pistol->SetVisibility(false);
	
}

void AFPCCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	//FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Assault_Rifle->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Pistol->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("Pistol_GripPoint"));
	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);

	
}





void AFPCCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPCCharacter::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPCCharacter::OnEndFire);

	// Bind Reload Event
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPCCharacter::Reload);

	// Bind weapon selection Events
	PlayerInputComponent->BindAction("WeaponSelectOne", IE_Pressed, this, &AFPCCharacter::WeaponSelectOne);
	PlayerInputComponent->BindAction("WeaponSelectTwo", IE_Pressed, this, &AFPCCharacter::WeaponSelectTwo);

	// Bind player crouching events
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFPCCharacter::Crouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AFPCCharacter::UnCrouch);

	PlayerInputComponent->BindAction("AimDownSights", IE_Pressed, this, &AFPCCharacter::AimDownSight);
	PlayerInputComponent->BindAction("AimDownSights", IE_Released, this, &AFPCCharacter::ReleaseAim);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPCCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPCCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPCCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPCCharacter::LookUpAtRate);
}

void AFPCCharacter::TimelineProgress(float Value)
{
	GetFirstPersonCameraComponent()->SetFieldOfView(FMath::FInterpTo(90,45.0f,50,Value));
}

void AFPCCharacter::OnFire()
{
	UWorld* const World = GetWorld();
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	UAnimInstance* PistolAnimInstance = Pistol->GetAnimInstance();
	UAnimInstance* AssaultRifleInstance = Assault_Rifle->GetAnimInstance();

	switch (weapon)
	{
	case assaultRifle:
		if (AssaultRifleAmmo > 0 && !IsReloading)
		{
			// try and fire a projectile
			if (ProjectileClass != nullptr)
			{
				if (World != nullptr)
				{
					if (IsADS)
					{
						GunOffset.Y = 2.0f;
						AssaultRifleRotation_Pitch = 0.0f;

						float RecoilUpDown = FMath::FRandRange(0, -.2);
						float RecoilLeftRight = FMath::FRandRange(-.01, .01);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
					else
					{
						GunOffset.Y = 13.0f;
						AssaultRifleRotation_Pitch = -2.0f;


						float RecoilUpDown = FMath::FRandRange(0, -.5);
						float RecoilLeftRight = FMath::FRandRange(-0.5, 0.5);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
						
					}
					
					const FRotator SpawnRotation = { GetControlRotation().Pitch + AssaultRifleRotation_Pitch,GetControlRotation().Yaw + AssaultRifleRotation_Yaw,GetControlRotation().Roll + AssaultRifleRotation_Roll};
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPCProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					AssaultRifleAmmo--;
					UGameplayStatics::SpawnEmitterAtLocation(World, AssaultRifle_MuzzleFlashParticle, SpawnLocation, GetActorRotation(), FVector(0.07, 0.07, 0.07));
				}
			}

			// try and play a firing animation if specified
			if (AssaultRifle_FireAnimation != nullptr)
			{
				// Get the animation object for the arms mesh
				if (AnimInstance != nullptr)
				{
					if (!IsADS)
					{
						AnimInstance->Montage_Play(AssaultRifle_FireAnimation, 2.5f);
					}
					else
					{
						AnimInstance->Montage_Play(AssaultRifle_ADS_FireAnimation, 2.5f);
					}
				}
			}
			if (AssaultRifleInstance != nullptr)
			{
				Assault_Rifle->PlayAnimation(AssaultRifle_Fire_FireAnimation, false);
			}
		}
		break;

	case pistol:
		if (PistolAmmo > 0 && !IsReloading)
		{
			// try and fire a projectile
			if (ProjectileClass != nullptr)
			{
				if (World != nullptr)
				{
					if (IsADS)
					{
						PistolOffset.Y = 2.0f;
						PistolRotation_Pitch = -2.5f;
						PistolRotation_Yaw = 0.3f;

						float RecoilUpDown = FMath::FRandRange(0, -.6);
						float RecoilLeftRight = FMath::FRandRange(-.3, .3);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
					else
					{
						PistolOffset.Y = 8.0f;
						PistolRotation_Pitch = -3.5f;
						PistolRotation_Yaw = -1.5f;

						float RecoilUpDown = FMath::FRandRange(0, -.9);
						float RecoilLeftRight = FMath::FRandRange(-1, 1);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}

					//const FRotator SpawnRotation = GetControlRotation();
					const FRotator SpawnRotation = {GetControlRotation().Pitch + PistolRotation_Pitch,GetControlRotation().Yaw + PistolRotation_Yaw,GetControlRotation().Roll + PistolRotation_Roll };
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					const FVector SpawnLocation = ((Pistol_FP_MuzzleLocation != nullptr) ? Pistol_FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(PistolOffset);

					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPCProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					PistolAmmo--;
					UGameplayStatics::SpawnEmitterAtLocation(World, Pistol_MuzzleFlashParticle, SpawnLocation, FRotator(GetActorRotation()), FVector(0.07, 0.07, 0.07));
					IsFiring = false;
				}
			}

			// try and play a firing animation if specified
			if (Pistol_FireAnimation != nullptr)
			{
				// Get the animation object for the arms mesh
				if (AnimInstance != nullptr)
				{
					if (!IsADS)
					{
						AnimInstance->Montage_Play(Pistol_FireAnimation, 2.5f);
					}
					else
					{
						AnimInstance->Montage_Play(Pistol_ADS_FireAnimation, 2.5f);
					}
	
				}
			}
			if (PistolAnimInstance != nullptr)
			{
				Pistol->PlayAnimation(Pistol_Fire_FireAnimation, false);
			}
		}
		break;
	}
	
}

void AFPCCharacter::OnBeginFire()
{
	switch (weapon)
	{
	case assaultRifle:
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &AFPCCharacter::OnFire, 0.1f, true, 0.01f);
		IsFiring = true;
		break;
	case pistol:
		OnFire();
		IsFiring = true;
		break;
	}	
}

void AFPCCharacter::OnEndFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	IsFiring = false;
}

void AFPCCharacter::Reload()
{
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	switch (weapon)
	{
	case assaultRifle:
		IsReloading = true;
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(AssaultRifle_ReloadingAnimation, 1.0f);
			
		}
		if (AssaultRifle_ReloadingAnimation_Montage != nullptr)
		{
			Assault_Rifle->PlayAnimation(AssaultRifle_ReloadingAnimation_Montage, false);
		}
		break;
	case pistol:
		IsReloading = true;
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(Pistol_ReloadingAnimation, 1.0f);
			
		}
		if (Pistol_ReloadingAnimation_Montage != nullptr)
		{
			Pistol->PlayAnimation(Pistol_ReloadingAnimation_Montage, false);
		}
		break;
	}
	
}

void AFPCCharacter::WeaponSelectOne()
{
	if (!IsReloading)
	{
	weapon = Weapons::assaultRifle;
	Pistol->SetVisibility(false);
	Assault_Rifle->SetVisibility(true);
	Mesh1P->SetAnimInstanceClass(AssaultRifle_AnimClass);
	}
}

void AFPCCharacter::WeaponSelectTwo()
{
	if (!IsReloading)
	{
		weapon = Weapons::pistol;
		Pistol->SetVisibility(true);
		Assault_Rifle->SetVisibility(false);
		Mesh1P->SetAnimInstanceClass(Pistol_AnimClass);
	}
}

void AFPCCharacter::Crouch()
{
	GetCapsuleComponent()->SetCapsuleHalfHeight(20.0f);
	GetFirstPersonCameraComponent()->SetRelativeLocation(FVector(-39.5f, 1.75f, 24.0f));
	GetCharacterMovement()->MaxWalkSpeed = 200;
}

void AFPCCharacter::UnCrouch()
{
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetFirstPersonCameraComponent()->SetRelativeLocation(FVector(-39.5f, 1.75f, 64.0f));
	GetCharacterMovement()->MaxWalkSpeed = 600;
}

void AFPCCharacter::AimDownSight()
{
	IsADS = true;
	GetCharacterMovement()->MaxWalkSpeed = 250.0f;
	AimDownBP();
}

void AFPCCharacter::ReleaseAim()
{
	IsADS = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	ReleaseAimBP();
}



void AFPCCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPCCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPCCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPCCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

