// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_LobbyGameMode.h"
#include "Framework/PEV2_LobbyPlayerController.h"
#include "Framework/PEV2_LobbyGameState.h"
#include "Core/PEV2_GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

APEV2_LobbyGameMode::APEV2_LobbyGameMode()
{
	// Deshabilitar Seamless Travel porque NO es soportado en Play In Editor (PIE)
	// Si está en true, ServerTravel fallará silenciosamente al jugar en el editor.
	bUseSeamlessTravel = false;

	PlayerControllerClass = APEV2_LobbyPlayerController::StaticClass();
	GameStateClass = APEV2_LobbyGameState::StaticClass();
	DefaultPawnClass = nullptr;
	GameArenaMapPath = TEXT("/Game/PEV2/Maps/L_GameArena");
	MinPlayersToStart = 1;
	ConnectedPlayers = 0;
}

void APEV2_LobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Transmitir el código de sala activo desde GameInstance hacia el GameState replicado
	if (UPEV2_GameInstance* GI = Cast<UPEV2_GameInstance>(GetGameInstance()))
	{
		if (APEV2_LobbyGameState* LobbyGS = GetGameState<APEV2_LobbyGameState>())
		{
			LobbyGS->SetRoomCode(GI->GetCurrentRoomCode());
		}
	}
}

void APEV2_LobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ConnectedPlayers++;

	FString PlayerName = TEXT("Jugador");
	if (NewPlayer && NewPlayer->PlayerState)
	{
		PlayerName = NewPlayer->PlayerState->GetPlayerName();
	}

	UE_LOG(LogTemp, Log, TEXT("[PEV2_LobbyGameMode] Nuevo jugador conectado: %s. Total en Lobby: %d"), *PlayerName, ConnectedPlayers);

	if (APEV2_LobbyGameState* LobbyGS = GetGameState<APEV2_LobbyGameState>())
	{
		LobbyGS->SetConnectedPlayerCount(ConnectedPlayers);
		if (UPEV2_GameInstance* GI = Cast<UPEV2_GameInstance>(GetGameInstance()))
		{
			LobbyGS->SetRoomCode(GI->GetCurrentRoomCode());
		}
	}

	OnLobbyPlayerCountChanged.Broadcast(ConnectedPlayers);
}

void APEV2_LobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ConnectedPlayers = FMath::Max(0, ConnectedPlayers - 1);

	UE_LOG(LogTemp, Log, TEXT("[PEV2_LobbyGameMode] Jugador desconectado. Total en Lobby: %d"), ConnectedPlayers);

	if (APEV2_LobbyGameState* LobbyGS = GetGameState<APEV2_LobbyGameState>())
	{
		LobbyGS->SetConnectedPlayerCount(ConnectedPlayers);
	}

	OnLobbyPlayerCountChanged.Broadcast(ConnectedPlayers);
}

void APEV2_LobbyGameMode::StartGame()
{
	// Verificación de autoridad: Solo el servidor/host puede ordenar el ServerTravel
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PEV2_LobbyGameMode] Intento no autorizado de iniciar partida desde un cliente."));
		return;
	}

	if (ConnectedPlayers < MinPlayersToStart)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PEV2_LobbyGameMode] No se puede iniciar: Jugadores conectados (%d) < Mínimo requerido (%d)"),
			ConnectedPlayers, MinPlayersToStart);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		// NO usar Seamless Travel en PIE
		bUseSeamlessTravel = false;

		FString TravelURL = FString::Printf(TEXT("%s?listen"), *GameArenaMapPath);
		UE_LOG(LogTemp, Log, TEXT("[PEV2_LobbyGameMode] Iniciando ServerTravel a Arena: %s"), *TravelURL);

		World->ServerTravel(TravelURL);
	}
}
