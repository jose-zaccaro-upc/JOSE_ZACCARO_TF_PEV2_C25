// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PEV2_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAliveStateChanged, bool, bIsAlive);

/**
 * PlayerState for Battle Zone.
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
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|Stats")
	int32 Eliminations;

	/** Time survived in the match */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|Stats")
	float SurvivalTime;

	// Server functions to modify stats
	void SetIsAlive(bool bNewAlive);
	void AddElimination();
};
