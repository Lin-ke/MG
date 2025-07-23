// Copyright Epic Games, Inc. All Rights Reserved.

#include "mg2GameMode.h"
#include "mg2Character.h"
#include "UObject/ConstructorHelpers.h"

Amg2GameMode::Amg2GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
