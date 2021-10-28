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
#include "FPCProjectile_AssaultRifle.h"
#include "FPCProjectile_Pistol.h"
#include "FPCProjectile_Shotgun.h"
#include "FPCProjectile_Rocket.h"
#include "FPCProjectile_Grenade.h"
#include "FPCProjectile_Sniper.h"
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

	Shot_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shot Gun"));
	Shot_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	Shot_Gun->bCastDynamicShadow = false;
	Shot_Gun->CastShadow = false;
	Shot_Gun->SetupAttachment(Mesh1P, TEXT("ShotGun_GripPoint"));
	Shot_Gun->SetupAttachment(RootComponent);

	Rocket_Launcher = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Rocket Launcher"));
	Rocket_Launcher->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	Rocket_Launcher->bCastDynamicShadow = false;
	Rocket_Launcher->CastShadow = false;
	Rocket_Launcher->SetupAttachment(Mesh1P, TEXT("RocketLauncher_GripSocket"));
	Rocket_Launcher->SetupAttachment(RootComponent);

	Grenade_Launcher = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Grenade Launcher"));
	Grenade_Launcher->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	Grenade_Launcher->bCastDynamicShadow = false;
	Grenade_Launcher->CastShadow = false;
	Grenade_Launcher->SetupAttachment(Mesh1P, TEXT("GrenadeLauncher_GripSocket"));
	Grenade_Launcher->SetupAttachment(RootComponent);

	Sniper_Rifle = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Sniper Rifle"));
	Sniper_Rifle->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	Sniper_Rifle->bCastDynamicShadow = false;
	Sniper_Rifle->CastShadow = false;
	Sniper_Rifle->SetupAttachment(Mesh1P, TEXT("Sniper_GripSocket"));
	Sniper_Rifle->SetupAttachment(RootComponent);

	Combat_Knife = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Combat Knife"));
	Combat_Knife->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	Combat_Knife->bCastDynamicShadow = false;
	Combat_Knife->CastShadow = false;
	Combat_Knife->SetupAttachment(Mesh1P, TEXT("Knife_GripSocket"));
	Combat_Knife->SetupAttachment(RootComponent);

	AssaultRifle_Ammo = 30;
	Pistol_Ammo = 15;
	Shotgun_Ammo = 8;
	Rocket_Ammo = 3;
	Sniper_Ammo = 10;
	GrenadeLauncher_Ammo = 6;

	IsReloading = false;
	IsFiring = false;
	weapon = Weapons::pistol;
	Pistol->SetVisibility(false);
	Shot_Gun->SetVisibility(false);
	Rocket_Launcher->SetVisibility(false);
	Grenade_Launcher->SetVisibility(false);
	Sniper_Rifle->SetVisibility(false);
	Combat_Knife->SetVisibility(false);
}

void AFPCCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	Assault_Rifle->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Pistol->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("Pistol_GripPoint"));
	Shot_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("ShotGun_GripPoint"));
	Rocket_Launcher->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("RocketLauncher_GripSocket"));
	Grenade_Launcher->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GrenadeLauncher_GripSocket"));
	Sniper_Rifle->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("Sniper_GripSocket"));
	Combat_Knife->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("Knife_GripSocket"));
	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);

	switch (weapon)
	{
	case assaultRifle:
		WeaponSelectOne();
		break;
	case pistol:
		WeaponSelectTwo();
		break;
	case shotgun:
		WeaponSelectThree();
		break;
	case rocket:
		WeaponSelectFour();
		break;
	case grenadeLauncher:
		WeaponSelectFive();
		break;
	case sniper:
		WeaponSelectSix();
		break;
	case knife:
		WeaponSelectSeven();
		break;
	}
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
	PlayerInputComponent->BindAction("WeaponSelectThree", IE_Pressed, this, &AFPCCharacter::WeaponSelectThree);
	PlayerInputComponent->BindAction("WeaponSelectFour", IE_Pressed, this, &AFPCCharacter::WeaponSelectFour);
	PlayerInputComponent->BindAction("WeaponSelectFive", IE_Pressed, this, &AFPCCharacter::WeaponSelectFive);
	PlayerInputComponent->BindAction("WeaponSelectSix", IE_Pressed, this, &AFPCCharacter::WeaponSelectSix);
	PlayerInputComponent->BindAction("WeaponSelectSeven", IE_Pressed, this, &AFPCCharacter::WeaponSelectSeven);
	

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
	UAnimInstance* RocketLauncherInstance = Rocket_Launcher->GetAnimInstance();
	UAnimInstance* GrenadeLauncherInstance = Grenade_Launcher->GetAnimInstance();
	UAnimInstance* SniperRifleInstance = Sniper_Rifle->GetAnimInstance();

	switch (weapon)
	{
	case assaultRifle:
		if (AssaultRifle_Ammo > 0 && !IsReloading)
		{
			// try and fire a projectile
			if (AssaultRifle_ProjectileClass != nullptr)
			{
				if (World != nullptr)
				{
					

					const FRotator SpawnRotation = { GetControlRotation().Pitch + AssaultRifle_Rotation_Pitch,GetControlRotation().Yaw + AssaultRifle_Rotation_Yaw,GetControlRotation().Roll + AssaultRifle_Rotation_Roll };
					const FVector SpawnLocation = Assault_Rifle->GetSocketLocation("ProjectileLocationSocket");
					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPCProjectile_AssaultRifle>(AssaultRifle_ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					AssaultRifle_Ammo--;

					if (IsADS)
					{
						AssaultRifle_Rotation_Pitch = 0.0f;

						float RecoilUpDown = FMath::FRandRange(0, -.2);
						float RecoilLeftRight = FMath::FRandRange(-.01, .01);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
					else
					{
						AssaultRifle_Rotation_Pitch = -2.0f;

						float RecoilUpDown = FMath::FRandRange(0, -.5);
						float RecoilLeftRight = FMath::FRandRange(-0.5, 0.5);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);

					}
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
		if (Pistol_Ammo > 0 && !IsReloading)
		{
			// try and fire a projectile
			if (Pistol_ProjectileClass != nullptr)
			{
				if (World != nullptr)
				{
					

					const FRotator SpawnRotation = { GetControlRotation().Pitch + Pistol_Rotation_Pitch,GetControlRotation().Yaw + Pistol_Rotation_Yaw,GetControlRotation().Roll + Pistol_Rotation_Roll };
					const FVector SpawnLocation = Pistol->GetSocketLocation("ProjectileLocationSocket");
					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPCProjectile_Pistol>(Pistol_ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					Pistol_Ammo--;
					IsFiring = false;

					if (IsADS)
					{
						Pistol_Rotation_Pitch = -2.5f;
						Pistol_Rotation_Yaw = 0.3f;

						float RecoilUpDown = FMath::FRandRange(0, -.6);
						float RecoilLeftRight = FMath::FRandRange(-.3, .3);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
					else
					{
						Pistol_Rotation_Pitch = -3.5f;
						Pistol_Rotation_Yaw = -1.5f;

						float RecoilUpDown = FMath::FRandRange(0, -.9);
						float RecoilLeftRight = FMath::FRandRange(-1, 1);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
				}
			}

			// try and play a firing animation if specified
			if (Pistol_FireAnimation != nullptr)
			{
				// Get the animation object for the arms mesh
				if (AnimInstance != nullptr)
				{
					AnimInstance->Montage_Play(Pistol_ADS_FireAnimation, 2.5f);
				}
			}
			if (PistolAnimInstance != nullptr)
			{
				Pistol->PlayAnimation(Pistol_Fire_FireAnimation, false);
			}
		}
		break;

	case shotgun:
		if (Shotgun_Ammo > 0 && !IsShotgunFiring)
		{
			if (IsReloading)
			{
				IsReloading = false;
			}
			IsShotgunFiring = true;
			// try and fire a projectile
			if (Shotgun_ProjectileClass != nullptr)
			{
				
				const FRotator SpawnRotationOne = { GetControlRotation().Pitch - 4.4f,GetControlRotation().Yaw + -0.65f, GetControlRotation().Roll};
				const FRotator SpawnRotationTwo = { GetControlRotation().Pitch - 4.4f,GetControlRotation().Yaw + -1.0f, GetControlRotation().Roll };
				const FRotator SpawnRotationThree = { GetControlRotation().Pitch - 4.4f,GetControlRotation().Yaw + -1.65f, GetControlRotation().Roll };
				const FRotator SpawnRotationFour = { GetControlRotation().Pitch - 4.4f,GetControlRotation().Yaw + 0.65f, GetControlRotation().Roll };
				const FVector SpawnLocation = Shot_Gun->GetSocketLocation("ProjectileLocationSocket");
				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AFPCProjectile_Shotgun>(Shotgun_ProjectileClass, SpawnLocation, SpawnRotationOne, ActorSpawnParams);
				World->SpawnActor<AFPCProjectile_Shotgun>(Shotgun_ProjectileClass, SpawnLocation, SpawnRotationTwo, ActorSpawnParams);
				World->SpawnActor<AFPCProjectile_Shotgun>(Shotgun_ProjectileClass, SpawnLocation, SpawnRotationThree, ActorSpawnParams);
				World->SpawnActor<AFPCProjectile_Shotgun>(Shotgun_ProjectileClass, SpawnLocation, SpawnRotationFour, ActorSpawnParams);
				Shotgun_Ammo--;

				if (World != nullptr)
				{
					Shotgun_Rotation_Pitch = -3.5f;
					Shotgun_Rotation_Yaw = -1.5f;

					float RecoilUpDown = FMath::FRandRange(0, -.9);
					float RecoilLeftRight = FMath::FRandRange(-.6, .6);

					AddControllerPitchInput(RecoilUpDown);
					AddControllerYawInput(RecoilLeftRight);
				}
			}

			// try and play a firing animation if specified
			if (Shotgun_FireAnimation != nullptr)
			{
				// Get the animation object for the arms mesh
				if (AnimInstance != nullptr)
				{
					if (!IsADS)
					{
						AnimInstance->Montage_Play(Shotgun_FireAnimation, 1.0f);
					}
					else
					{
						AnimInstance->Montage_Play(Shotgun_ADS_FireAnimation, 1.0f);
					}
				}
			}
			if (ShotGun_AnimClass != nullptr)
			{
				Shot_Gun->PlayAnimation(Shotgun_Fire_FireAnimation, false);
			}
		}
		break;
	case rocket:
		if (Rocket_Ammo > 0 && !IsReloading && !IsShotgunFiring)
		{
			// try and fire a projectile
			if (Rocket_ProjectileClass != nullptr)
			{
				IsShotgunFiring = true;
				if (World != nullptr)
				{
					const FRotator SpawnRotation = { GetControlRotation().Pitch - 1.2f,GetControlRotation().Yaw + 0.3f ,GetControlRotation().Roll };
					const FVector SpawnLocation = Rocket_Launcher->GetSocketLocation("ProjectileLocationSocket");
					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPCProjectile_Rocket>(Rocket_ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					Rocket_Ammo--;
					IsFiring = false;

					if (IsADS)
					{
						float RecoilUpDown = FMath::FRandRange(0, -.6);
						float RecoilLeftRight = FMath::FRandRange(-.3, .3);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
					else
					{
						float RecoilUpDown = FMath::FRandRange(0, -.9);
						float RecoilLeftRight = FMath::FRandRange(-1, 1);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
				}
			}

			// try and play a firing animation if specified
			if (IsADS)
			{
				if (Rocket_FireAnimation != nullptr)
				{
					// Get the animation object for the arms mesh
					if (AnimInstance != nullptr)
					{
						AnimInstance->Montage_Play(Rocket_ADS_FireAnimation, 2.5f);
					}
				}
			}
			else
			{
				if (Rocket_FireAnimation != nullptr)
				{
					// Get the animation object for the arms mesh
					if (AnimInstance != nullptr)
					{
						AnimInstance->Montage_Play(Rocket_FireAnimation, 2.5f);
					}
				}
			}
			if (Rocket_AnimClass != nullptr)
			{
				Rocket_Launcher->PlayAnimation(Rocket_Fire_FireAnimation, false);
			}
		}
		break;
	case grenadeLauncher:
		if (GrenadeLauncher_Ammo > 0 && !IsReloading && !IsShotgunFiring)
		{
			// try and fire a projectile
			if (Grenade_ProjectileClass != nullptr)
			{
				IsShotgunFiring = true;
				if (World != nullptr)
				{
					const FRotator SpawnRotation = { GetControlRotation().Pitch - 1.2f,GetControlRotation().Yaw + 0.3f ,GetControlRotation().Roll };
					const FVector SpawnLocation = Grenade_Launcher->GetSocketLocation("ProjectileLocationSocket");
					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPCProjectile_Grenade>(Grenade_ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					GrenadeLauncher_Ammo--;
					IsFiring = false;

					if (IsADS)
					{
						float RecoilUpDown = FMath::FRandRange(0, -.6);
						float RecoilLeftRight = FMath::FRandRange(-.3, .3);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
					else
					{
						float RecoilUpDown = FMath::FRandRange(0, -.9);
						float RecoilLeftRight = FMath::FRandRange(-1, 1);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
				}
			}

			// try and play a firing animation if specified
			if (IsADS)
			{
				if (Rocket_FireAnimation != nullptr)
				{
					// Get the animation object for the arms mesh
					if (AnimInstance != nullptr)
					{
						AnimInstance->Montage_Play(Rocket_ADS_FireAnimation, 2.5f);
					}
				}
			}
			else
			{
				if (Rocket_FireAnimation != nullptr)
				{
					// Get the animation object for the arms mesh
					if (AnimInstance != nullptr)
					{
						AnimInstance->Montage_Play(Rocket_FireAnimation, 2.5f);
					}
				}
			}
			if (Rocket_AnimClass != nullptr)
			{
				Grenade_Launcher->PlayAnimation(GrenadeLauncher_Fire_FireAnimation, false);
			}
		}
		break;
	case sniper:
		if (Sniper_Ammo > 0 && !IsReloading)
		{
			// try and fire a projectile
			if (Sniper_ProjectileClass != nullptr)
			{
				if (World != nullptr)
				{
					const FRotator SpawnRotation = { GetControlRotation().Pitch,GetControlRotation().Yaw,GetControlRotation().Roll};
					const FVector SpawnLocation = Sniper_Rifle->GetSocketLocation("ProjectileLocationSocket");
					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPCProjectile_Sniper>(Sniper_ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					Sniper_Ammo--;

					if (IsADS)
					{
						float RecoilUpDown = FMath::FRandRange(0, -.2);
						float RecoilLeftRight = FMath::FRandRange(-.01, .01);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);
					}
					else
					{
						float RecoilUpDown = FMath::FRandRange(0, -.5);
						float RecoilLeftRight = FMath::FRandRange(-0.5, 0.5);

						AddControllerPitchInput(RecoilUpDown);
						AddControllerYawInput(RecoilLeftRight);

					}
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
			if (SniperRifleInstance != nullptr)
			{
				Sniper_Rifle->PlayAnimation(SniperRifle_Fire_FireAnimation, false);
			}
		}
		break;
	case knife:
		if (!IsKnifeSwinging)
		{
			IsKnifeSwinging = true;
			int ranNum = FMath::RandRange(1, 2);
			switch (ranNum)
			{
			case 1:
				if (AnimInstance != nullptr)
				{
					AnimInstance->Montage_Play(Knife_Stab_One_Animation, 1.0f);
				}
				break;
			case 2:
				if (AnimInstance != nullptr)
				{
					AnimInstance->Montage_Play(Knife_Stab_Two_Animation, 1.0f);
				}
				break;
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
	case shotgun:
		OnFire();
		IsFiring = true;
		break;
	case rocket:
		OnFire();
		IsFiring = true;
		break;
	case grenadeLauncher:
		OnFire();
		IsFiring = true;
		break;
	case sniper:
		OnFire();
		IsFiring = true;
		break;
	case knife:
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
	if (!IsReloading)
	{
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
		case shotgun:
			if (Shotgun_Ammo < 9 && !IsReloading && !IsShotgunFiring)
			{
				IsReloading = true;
			}
			break;
		case rocket:
			IsReloading = true;
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Play(Rocket_ReloadingAnimation, 1.0f);
			}
			break;
		case grenadeLauncher:
			IsReloading = true;
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Play(Rocket_ReloadingAnimation, 1.0f);
			}
			break;
		case sniper:
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Play(AssaultRifle_ReloadingAnimation, 1.0f);
			}
			if (AssaultRifle_ReloadingAnimation_Montage != nullptr)
			{
				Sniper_Rifle->PlayAnimation(SniperRifle_Reloading_Animation, false);
			}
			break;
		}
	}
}

void AFPCCharacter::WeaponSelectOne()
{
	if (!IsReloading && !IsADS && !IsFiring && !IsShotgunFiring)
	{
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalOffset(FVector(-5.0f, 15.0f, -13.0f));
		}

		weapon = Weapons::assaultRifle;
		Assault_Rifle->SetVisibility(true);
		Assault_Rifle->bPauseAnims = false;

		Pistol->SetVisibility(false);
		Pistol->bPauseAnims = true;

		Rocket_Launcher->SetVisibility(false);
		Rocket_Launcher->bPauseAnims = true;

		Grenade_Launcher->bPauseAnims = true;
		Grenade_Launcher->SetVisibility(false);

		Shot_Gun->bPauseAnims = true;
		Shot_Gun->SetVisibility(false);

		Combat_Knife->bPauseAnims = true;
		Combat_Knife->SetVisibility(false);

		Sniper_Rifle->SetVisibility(false);
		Sniper_Rifle->bPauseAnims = true;

		if (AssaultRifle_AnimClass != nullptr)
		{
			Mesh1P->SetAnimInstanceClass(AssaultRifle_AnimClass);
		}
	}
}

void AFPCCharacter::WeaponSelectTwo()
{
	if (!IsReloading && !IsADS && !IsFiring && !IsShotgunFiring)
	{
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalOffset(FVector(-5.0f, 15.0f, -13.0f));
		}
		weapon = Weapons::pistol;
		Pistol->SetVisibility(true);
		Pistol->bPauseAnims = false;

		Rocket_Launcher->SetVisibility(false);
		Rocket_Launcher->bPauseAnims = true;

		Assault_Rifle->bPauseAnims = true;
		Shot_Gun->bPauseAnims = true;

		Grenade_Launcher->bPauseAnims = true;
		Grenade_Launcher->SetVisibility(false);

		Sniper_Rifle->SetVisibility(false);
		Sniper_Rifle->bPauseAnims = true;

		Combat_Knife->bPauseAnims = true;
		Combat_Knife->SetVisibility(false);

		Shot_Gun->SetVisibility(false);
		Assault_Rifle->SetVisibility(false);

		if (Pistol_AnimClass != nullptr)
		{
			Mesh1P->SetAnimInstanceClass(Pistol_AnimClass);
		}
	}
}

void AFPCCharacter::WeaponSelectThree()
{
	if (!IsReloading && !IsADS && !IsFiring && !IsShotgunFiring)
	{
		if (weapon != shotgun)
		{
			Mesh1P->AddLocalOffset(FVector(5.0f, -15.0f, 13.0f));
		}
		weapon = Weapons::shotgun;
		Shot_Gun->bPauseAnims = false;

		Pistol->SetVisibility(false);
		Pistol->bPauseAnims = true;

		Rocket_Launcher->SetVisibility(false);
		Rocket_Launcher->bPauseAnims = true;

		Assault_Rifle->bPauseAnims = true;
		Assault_Rifle->SetVisibility(false);

		Sniper_Rifle->SetVisibility(false);
		Sniper_Rifle->bPauseAnims = true;

		Combat_Knife->bPauseAnims = true;
		Combat_Knife->SetVisibility(false);

		Grenade_Launcher->bPauseAnims = true;
		Grenade_Launcher->SetVisibility(false);


		Shot_Gun->SetVisibility(true);

		if (ShotGun_AnimClass != nullptr)
		{
			Mesh1P->SetAnimInstanceClass(ShotGun_AnimClass);
		}
	}
}

void AFPCCharacter::WeaponSelectFour()
{
	if (!IsReloading && !IsADS && !IsFiring && !IsShotgunFiring)
	{
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalOffset(FVector(-5.0f, 15.0f, -13.0f));
		}
		weapon = Weapons::rocket;

		Pistol->SetVisibility(false);
		Pistol->bPauseAnims = true;

		Assault_Rifle->bPauseAnims = true;
		Assault_Rifle->SetVisibility(false);

		Shot_Gun->SetVisibility(false);
		Shot_Gun->bPauseAnims = true;

		Sniper_Rifle->SetVisibility(false);
		Sniper_Rifle->bPauseAnims = true;

		Grenade_Launcher->bPauseAnims = true;
		Grenade_Launcher->SetVisibility(false);

		Combat_Knife->bPauseAnims = true;
		Combat_Knife->SetVisibility(false);

		Rocket_Launcher->bPauseAnims = false;
		Rocket_Launcher->SetVisibility(true);

		if (Rocket_AnimClass != nullptr)
		{
			Mesh1P->SetAnimInstanceClass(Rocket_AnimClass);
		}
	}
}

void AFPCCharacter::WeaponSelectFive()
{
	if (!IsReloading && !IsADS && !IsFiring && !IsShotgunFiring)
	{
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalOffset(FVector(-5.0f, 15.0f, -13.0f));
		}
		weapon = Weapons::grenadeLauncher;

		Pistol->SetVisibility(false);
		Pistol->bPauseAnims = true;

		Assault_Rifle->bPauseAnims = true;
		Assault_Rifle->SetVisibility(false);

		Shot_Gun->SetVisibility(false);
		Shot_Gun->bPauseAnims = true;

		Rocket_Launcher->bPauseAnims = true;
		Rocket_Launcher->SetVisibility(false);

		Sniper_Rifle->SetVisibility(false);
		Sniper_Rifle->bPauseAnims = true;

		Combat_Knife->bPauseAnims = true;
		Combat_Knife->SetVisibility(false);

		Grenade_Launcher->bPauseAnims = false;
		Grenade_Launcher->SetVisibility(true);

		if (Rocket_AnimClass != nullptr)
		{
			 Mesh1P->SetAnimInstanceClass(Rocket_AnimClass);
		}
	}
}

void AFPCCharacter::WeaponSelectSix()
{
	if (!IsReloading && !IsADS && !IsFiring && !IsShotgunFiring)
	{
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalOffset(FVector(-5.0f, 15.0f, -13.0f));
		}
		weapon = Weapons::sniper;

		Pistol->SetVisibility(false);
		Pistol->bPauseAnims = true;

		Assault_Rifle->bPauseAnims = true;
		Assault_Rifle->SetVisibility(false);

		Shot_Gun->SetVisibility(false);
		Shot_Gun->bPauseAnims = true;

		Rocket_Launcher->bPauseAnims = true;
		Rocket_Launcher->SetVisibility(false);

		Grenade_Launcher->bPauseAnims = true;
		Grenade_Launcher->SetVisibility(false);

		Combat_Knife->bPauseAnims = true;
		Combat_Knife->SetVisibility(false);

		Sniper_Rifle->bPauseAnims = false;
		Sniper_Rifle->SetVisibility(true);

		if (AssaultRifle_AnimClass != nullptr)
		{
			Mesh1P->SetAnimInstanceClass(AssaultRifle_AnimClass);
		}
	}

}

void AFPCCharacter::WeaponSelectSeven()
{
	if (!IsReloading && !IsADS && !IsFiring && !IsShotgunFiring)
	{
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalOffset(FVector(-5.0f, 15.0f, -13.0f));
		}
		weapon = Weapons::knife;

		Pistol->SetVisibility(false);
		Pistol->bPauseAnims = true;

		Assault_Rifle->bPauseAnims = true;
		Assault_Rifle->SetVisibility(false);

		Shot_Gun->SetVisibility(false);
		Shot_Gun->bPauseAnims = true;

		Rocket_Launcher->bPauseAnims = true;
		Rocket_Launcher->SetVisibility(false);

		Grenade_Launcher->bPauseAnims = true;
		Grenade_Launcher->SetVisibility(false);

		Sniper_Rifle->bPauseAnims = true;
		Sniper_Rifle->SetVisibility(false);

		Combat_Knife->bPauseAnims = false;
		Combat_Knife->SetVisibility(true);

		if (Knife_AnimClass != nullptr)
		{
			Mesh1P->SetAnimInstanceClass(Knife_AnimClass);
		}
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
	if (weapon != knife)
	{
		IsADS = true;
		GetCharacterMovement()->MaxWalkSpeed = 250.0f;
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalRotation(FRotator(0.0f, -0.5f, 0.0f));
		}
		else if (weapon == rocket)
		{
			Rocket_Launcher->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("RocketLauncher_GripSocket_ADS"));
		}
		else if (weapon == grenadeLauncher)
		{
			Grenade_Launcher->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GrenadeLauncher_GripSocket_ADS"));
		}
		else if (weapon == sniper)
		{
			Sniper_Rifle->SetVisibility(false);
			Mesh1P->SetVisibility(false);
		}
		AimDownBP();
		}
}

void AFPCCharacter::ReleaseAim()
{
	if (weapon != knife)
	{
		IsADS = false;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		if (weapon == shotgun)
		{
			Mesh1P->AddLocalRotation(FRotator(0.0f, 0.5f, 0.0f));
		}
		else if (weapon == rocket)
		{
			Rocket_Launcher->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("RocketLauncher_GripSocket"));
		}
		else if (weapon == grenadeLauncher)
		{
			Grenade_Launcher->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GrenadeLauncher_GripSocket"));
		}
		else if (weapon == sniper)
		{
			Sniper_Rifle->SetVisibility(true);
			Mesh1P->SetVisibility(true);
		}

		ReleaseAimBP();
	}
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

