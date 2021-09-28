// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPCGameMode.h"
#include "FPCHUD.h"
#include "FPCCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPCGameMode::AFPCGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPCHUD::StaticClass();
}
