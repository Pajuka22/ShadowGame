// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ShadowGameGameMode.h"
#include "ShadowGameHUD.h"
#include "ShadowGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AShadowGameGameMode::AShadowGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AShadowGameHUD::StaticClass();
}
