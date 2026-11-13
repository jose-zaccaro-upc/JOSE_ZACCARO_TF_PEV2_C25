#include "Framework/PEV2_GameMode.h"
#include "Framework/PEV2_GameState.h"
#include "Framework/PEV2_PlayerState.h"
#include "Framework/PEV2_PlayerController.h"
#include "Characters/PEV2_Character.h"
#include "Gameplay/Weapons/PEV2_ProjectileBase.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "Gameplay/Weapons/PEV2_WeaponRoster.h"
#include "UI/PEV2_HUD.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

APEV2_GameMode::APEV2_GameMode()
{
	GameStateClass = APEV2_GameState::StaticClass();
	PlayerStateClass = APEV2_PlayerState::StaticClass();
	HUDClass = APEV2_HUD::StaticClass();
	KillsToWin = 10;
	FinalWeaponTierIndex = 4; // Cuchillo de combate
	RespawnDelay = 3.0f;
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
		if (PS->CharacterAppearanceIndex < 0)
		{
			PS->CharacterAppearanceIndex = FMath::RandRange(0, 3);
		}
		
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
	CheckWinCondition(nullptr, nullptr);
}

void APEV2_GameMode::OnPlayerKilled(AController* VictimController, AController* KillerController, AActor* DamageCauser)
{
	if (VictimController)
	{
		if (APEV2_PlayerState* VictimPS = VictimController->GetPlayerState<APEV2_PlayerState>())
		{
			VictimPS->SetIsAlive(false);
		}

		// Programar la reaparición del jugador eliminado en la arena tras RespawnDelay segundos
		FTimerHandle RespawnTimerHandle;
		FTimerDelegate RespawnDelegate;
		RespawnDelegate.BindUObject(this, &APEV2_GameMode::RespawnPlayer, VictimController);
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);
	}

	if (KillerController && KillerController != VictimController)
	{
		if (APEV2_PlayerState* KillerPS = KillerController->GetPlayerState<APEV2_PlayerState>())
		{
			KillerPS->AddElimination();

			// Determinar si la baja fue provocada por una habilidad táctica (ej. granada arrojadiza de habilidad) y no por un arma principal.
			// Las bajas con armas principales del roster (incluyendo el Lanzagranadas) sí hacen avanzar el nivel de arma.
			bool bIsAbilityGrenadeKill = false;
			if (DamageCauser)
			{
				if (APEV2_ProjectileBase* Proj = Cast<APEV2_ProjectileBase>(DamageCauser))
				{
					if (Proj->IsAbilityProjectile())
					{
						bIsAbilityGrenadeKill = true;
					}
				}
				else if (!DamageCauser->IsA<APEV2_WeaponBase>() && DamageCauser->GetName().ToLower().Contains(TEXT("ability")))
				{
					bIsAbilityGrenadeKill = true;
				}
			}

			if (!bIsAbilityGrenadeKill)
			{
				if (APEV2_Character* KillerChar = Cast<APEV2_Character>(KillerController->GetPawn()))
				{
					KillerChar->AdvanceWeaponTier();
				}

				CheckWinCondition(KillerController, DamageCauser);
			}

			// Notificar al atacante mediante Client RPC para UI y audio local de baja
			if (APEV2_PlayerController* KillerPC = Cast<APEV2_PlayerController>(KillerController))
			{
				KillerPC->Client_NotifyKillScored(KillerPS->Eliminations, KillsToWin);
			}
		}
	}
}

void APEV2_GameMode::RespawnPlayer(AController* Controller)
{
	if (!Controller || HasMatchEnded())
	{
		return;
	}

	if (APawn* OldPawn = Controller->GetPawn())
	{
		OldPawn->Destroy();
	}

	RestartPlayer(Controller);
}

void APEV2_GameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	if (APEV2_PlayerState* PS = NewPlayer->GetPlayerState<APEV2_PlayerState>())
	{
		PS->SetIsAlive(true);
		if (PS->CharacterAppearanceIndex < 0)
		{
			PS->CharacterAppearanceIndex = FMath::RandRange(0, 3);
		}

		if (APEV2_Character* NewChar = Cast<APEV2_Character>(NewPlayer->GetPawn()))
		{
			NewChar->RestoreFromPlayerState(PS);
		}
	}
	else if (APEV2_Character* NewChar = Cast<APEV2_Character>(NewPlayer->GetPawn()))
	{
		NewChar->ResetForRespawn();
	}
}

void APEV2_GameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void APEV2_GameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	
	UE_LOG(LogTemp, Log, TEXT("Arms Race Arena match has ended! Triggering Victory/Defeat screens."));

	// Identificar al ganador con más eliminaciones
	AController* WinningController = nullptr;
	int32 HighestKills = -1;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (APEV2_PlayerState* PS = PC->GetPlayerState<APEV2_PlayerState>())
			{
				if (PS->Eliminations > HighestKills)
				{
					HighestKills = PS->Eliminations;
					WinningController = PC;
				}
			}
		}
	}

	// Enviar RPC de fin de partida a todos los PlayerControllers conectados
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (APEV2_PlayerController* PEV2_PC = Cast<APEV2_PlayerController>(PC))
			{
				const bool bIsWinner = (PC == WinningController);
				PEV2_PC->Client_ShowMatchEndScreen(bIsWinner);
			}
		}
	}
}

void APEV2_GameMode::CheckWinCondition(AController* PotentialWinner, AActor* DamageCauser)
{
	if (!HasMatchStarted() || HasMatchEnded())
	{
		return;
	}

	if (PotentialWinner)
	{
		if (APEV2_PlayerState* PS = PotentialWinner->GetPlayerState<APEV2_PlayerState>())
		{
			bool bWonMatch = false;

			// Condición 1: Alcanzó la cantidad de kills requeridas
			if (PS->Eliminations >= KillsToWin)
			{
				bWonMatch = true;
			}

			// Condición 2: Mató con el arma final (Cuchillo de combate)
			if (APEV2_Character* WinnerChar = Cast<APEV2_Character>(PotentialWinner->GetPawn()))
			{
				if (WinnerChar->GetCurrentWeaponTier() >= FinalWeaponTierIndex)
				{
					if (DamageCauser && (DamageCauser->IsA<APEV2_Weapon_Knife>() || DamageCauser->GetName().ToLower().Contains(TEXT("knife"))))
					{
						bWonMatch = true;
					}
				}
			}

			if (bWonMatch)
			{
				EndMatch();
			}
		}
	}
}
