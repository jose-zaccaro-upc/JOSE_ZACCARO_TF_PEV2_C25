// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_PlayerState.h"
#include "Net/UnrealNetwork.h"

APEV2_PlayerState::APEV2_PlayerState()
{
	bIsAlive = false;
	Eliminations = 0;
	SurvivalTime = 0.0f;
	SavedWeaponTier = 0;
	CharacterAppearanceIndex = -1;
}

void APEV2_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APEV2_PlayerState, bIsAlive);
	DOREPLIFETIME(APEV2_PlayerState, Eliminations);
	DOREPLIFETIME(APEV2_PlayerState, SurvivalTime);
	DOREPLIFETIME(APEV2_PlayerState, SavedWeaponTier);
	DOREPLIFETIME(APEV2_PlayerState, CharacterAppearanceIndex);
}

void APEV2_PlayerState::OnRep_bIsAlive()
{
	OnPlayerAliveStateChangedEvent.Broadcast(bIsAlive);
}

void APEV2_PlayerState::OnRep_Eliminations()
{
	OnEliminationsChangedEvent.Broadcast(Eliminations);
}

void APEV2_PlayerState::SetIsAlive(bool bNewAlive)
{
	if (HasAuthority())
	{
		bIsAlive = bNewAlive;
		OnRep_bIsAlive(); // Trigger on server too
	}
}

void APEV2_PlayerState::AddElimination()
{
	if (HasAuthority())
	{
		Eliminations++;
		OnRep_Eliminations();
	}
}
