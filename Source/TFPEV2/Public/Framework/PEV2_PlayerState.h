// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PEV2_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAliveStateChanged, bool, bIsAlive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEliminationsChanged, int32, NewEliminations);

/**
 * PlayerState for Battle Zone / Arms Race Arena.
 * Tracks eliminations, survival time, and alive status.
 */
UCLASS()
class TFPEV2_API APEV2_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	APEV2_PlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Is the player currently alive? */
	UPROPERTY(ReplicatedUsing = OnRep_bIsAlive, BlueprintReadOnly, Category = "PEV2|Stats")
	bool bIsAlive;

	UFUNCTION()
	void OnRep_bIsAlive();

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnPlayerAliveStateChanged OnPlayerAliveStateChangedEvent;

	/** Number of players this player has eliminated */
	UPROPERTY(ReplicatedUsing = OnRep_Eliminations, BlueprintReadOnly, Category = "PEV2|Stats")
	int32 Eliminations;

	UFUNCTION()
	void OnRep_Eliminations();

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnEliminationsChanged OnEliminationsChangedEvent;

	/** Time survived in the match */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|Stats")
	float SurvivalTime;

	/** Nivel de arma actual persistido a nivel de jugador para que no se pierda al morir */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|ArmsRace")
	int32 SavedWeaponTier;

	/** Índice de apariencia aleatoria asignado al jugador (para mallas y variaciones) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|Appearance")
	int32 CharacterAppearanceIndex;

	// Server functions to modify stats
	void SetIsAlive(bool bNewAlive);
	void AddElimination();
};
