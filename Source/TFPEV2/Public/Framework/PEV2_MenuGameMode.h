// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PEV2_MenuGameMode.generated.h"

/**
 * GameMode para el Menú Principal (L_MainMenu).
 * Desactiva el spawn de peones innecesarios y utiliza PEV2_MenuPlayerController.
 */
UCLASS()
class TFPEV2_API APEV2_MenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APEV2_MenuGameMode();
};
