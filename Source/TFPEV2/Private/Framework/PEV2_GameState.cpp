// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_GameState.h"
#include "Net/UnrealNetwork.h"

APEV2_GameState::APEV2_GameState()
{
	AlivePlayers = 0;
	ZoneCloseTime = 0.0f;
	ZonePhase = TEXT("None");
	bReplicates = true;
}

void APEV2_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APEV2_GameState, AlivePlayers);
	DOREPLIFETIME(APEV2_GameState, ZoneCloseTime);
	DOREPLIFETIME(APEV2_GameState, ZonePhase);
}

void APEV2_GameState::OnRep_AlivePlayers()
{
	OnAlivePlayersChangedEvent.Broadcast(AlivePlayers);
}

void APEV2_GameState::AddAlivePlayer()
{
	if (HasAuthority())
	{
		AlivePlayers++;
		OnRep_AlivePlayers(); // Trigger on server too
	}
}

void APEV2_GameState::RemoveAlivePlayer()
{
	if (HasAuthority())
	{
		AlivePlayers--;
		if (AlivePlayers < 0) AlivePlayers = 0;
		OnRep_AlivePlayers(); // Trigger on server too
	}
}
