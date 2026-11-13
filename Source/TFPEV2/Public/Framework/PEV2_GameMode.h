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

	/** Reaparece al jugador en un PlayerStart de la arena tras la muerte */
	UFUNCTION()
	void RespawnPlayer(AController* Controller);

protected:
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	/** Bajas requeridas para ganar la partida (por defecto 10 según la especificación de Arms Race) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Rules")
	int32 KillsToWin = 10;

	/** Nivel de arma del roster que se considera arma final para ganar (ej. 4 = Cuchillo de combate) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Rules")
	int32 FinalWeaponTierIndex = 4;

	/** Tiempo en segundos antes de reaparecer en la arena tras morir */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Rules")
	float RespawnDelay = 3.0f;

	/** Checks if win condition is met */
	void CheckWinCondition(AController* PotentialWinner, AActor* DamageCauser);

private:
	APEV2_GameState* GetPEV2GameState() const;
};
