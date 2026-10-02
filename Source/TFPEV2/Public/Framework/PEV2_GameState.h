// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PEV2_GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlivePlayersChanged, int32, NewAlivePlayers);

/**
 * GameState for the Battle Zone mode.
 * Replicates Match state, zone timers, and remaining players.
 */
UCLASS()
class TFPEV2_API APEV2_GameState : public AGameState
{
	GENERATED_BODY()

public:
	APEV2_GameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Number of players still alive in the match */
	UPROPERTY(ReplicatedUsing = OnRep_AlivePlayers, BlueprintReadOnly, Category = "PEV2|Match")
	int32 AlivePlayers;

	UFUNCTION()
	void OnRep_AlivePlayers();

	/** Notifies clients when players change */
	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnAlivePlayersChanged OnAlivePlayersChangedEvent;

	/** Total time before the zone finishes closing in the current phase */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|Zone")
	float ZoneCloseTime;

	/** Current phase of the zone (e.g., Delay, Shrinking) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|Zone")
	FName ZonePhase;

	// Helper to add/remove alive players
	void AddAlivePlayer();
	void RemoveAlivePlayer();
};
