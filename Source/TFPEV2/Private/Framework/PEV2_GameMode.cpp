// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_GameMode.h"
#include "Framework/PEV2_GameState.h"
#include "Framework/PEV2_PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

APEV2_GameMode::APEV2_GameMode()
{
	GameStateClass = APEV2_GameState::StaticClass();
	PlayerStateClass = APEV2_PlayerState::StaticClass();
}

APEV2_GameState* APEV2_GameMode::GetPEV2GameState() const
{
	return GetGameState<APEV2_GameState>();
}

void APEV2_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Register player in GameState
	if (APEV2_PlayerState* PS = NewPlayer->GetPlayerState<APEV2_PlayerState>())
	{
		PS->SetIsAlive(true);
		
		if (APEV2_GameState* GS = GetPEV2GameState())
		{
			GS->AddAlivePlayer();
		}
	}
}

void APEV2_GameMode::Logout(AController* Exiting)
{
	if (APEV2_PlayerState* PS = Exiting->GetPlayerState<APEV2_PlayerState>())
	{
		if (PS->bIsAlive)
		{
			PS->SetIsAlive(false);
			if (APEV2_GameState* GS = GetPEV2GameState())
			{
				GS->RemoveAlivePlayer();
			}
		}
	}

	Super::Logout(Exiting);
	CheckWinCondition();
}

void APEV2_GameMode::OnPlayerKilled(AController* VictimController, AController* KillerController, AActor* DamageCauser)
{
	if (VictimController)
	{
		if (APEV2_PlayerState* VictimPS = VictimController->GetPlayerState<APEV2_PlayerState>())
		{
			VictimPS->SetIsAlive(false);
			
			if (APEV2_GameState* GS = GetPEV2GameState())
			{
				GS->RemoveAlivePlayer();
			}
		}
	}

	if (KillerController && KillerController != VictimController)
	{
		if (APEV2_PlayerState* KillerPS = KillerController->GetPlayerState<APEV2_PlayerState>())
		{
			KillerPS->AddElimination();
		}
	}

	CheckWinCondition();
}

void APEV2_GameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	// Initialize zone contraction here if needed
}

void APEV2_GameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	
	// Show end screen, travel to lobby after delay
	UE_LOG(LogTemp, Log, TEXT("Match has ended!"));
}

void APEV2_GameMode::CheckWinCondition()
{
	if (!HasMatchStarted() || HasMatchEnded()) return;

	if (APEV2_GameState* GS = GetPEV2GameState())
	{
		// Last Man Standing condition
		if (GS->AlivePlayers <= 1)
		{
			EndMatch();
		}
	}
}
