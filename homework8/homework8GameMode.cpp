// Copyright Epic Games, Inc. All Rights Reserved.

#include "homework8GameMode.h"
#include "homework8Character.h"
#include "UObject/ConstructorHelpers.h"
#include "SpartaGameStateBase.h"

Ahomework8GameMode::Ahomework8GameMode()
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
    GameStateClass = ASpartaGameStateBase::StaticClass();
}
