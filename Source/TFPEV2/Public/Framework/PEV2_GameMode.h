// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PEV2_GameMode.generated.h"

class APEV2_GameState;
class APEV2_PlayerState;

/**
 * Core GameMode for the Battle Zone match.
 * Handles spawns, round lifecycle, and win conditions.
 */
UCLASS()
class TFPEV2_API APEV2_GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	APEV2_GameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Called when a player is killed */
	virtual void OnPlayerKilled(AController* VictimController, AController* KillerController, AActor* DamageCauser);

protected:
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

	/** Checks if only one player is remaining */
	void CheckWinCondition();

private:
	APEV2_GameState* GetPEV2GameState() const;
};
