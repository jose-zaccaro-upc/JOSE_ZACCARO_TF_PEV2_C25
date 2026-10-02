// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_MenuGameMode.h"
#include "Framework/PEV2_MenuPlayerController.h"

APEV2_MenuGameMode::APEV2_MenuGameMode()
{
	PlayerControllerClass = APEV2_MenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
