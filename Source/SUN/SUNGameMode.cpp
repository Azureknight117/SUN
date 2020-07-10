// Copyright Epic Games, Inc. All Rights Reserved.

#include "SUNGameMode.h"
#include "SUNHUD.h"
#include "SUNCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASUNGameMode::ASUNGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASUNHUD::StaticClass();
}
