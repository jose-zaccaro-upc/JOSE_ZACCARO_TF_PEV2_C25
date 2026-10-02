// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PEV2_LobbyGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerCountChanged, int32, NewPlayerCount);

/**
 * GameMode para la sala de espera (L_Lobby).
 * Maneja la conexión de jugadores, cuenta de participantes y Seamless ServerTravel hacia la Arena.
 */
UCLASS()
class TFPEV2_API APEV2_LobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APEV2_LobbyGameMode();

	// Notificaciones de ciclo de vida de jugadores en el servidor
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/**
	 * Inicia la transición a la Arena de combate (Solo ejecutable por el Servidor / Host).
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Lobby")
	void StartGame();

	/**
	 * Retorna la cantidad de jugadores conectados actualmente en el Lobby.
	 */
	UFUNCTION(BlueprintPure, Category = "PEV2|Lobby")
	int32 GetConnectedPlayerCount() const { return ConnectedPlayers; }

public:
	/** Ruta del mapa de la Arena donde se transfiere la partida */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PEV2|Lobby")
	FString GameArenaMapPath;

	/** Mínimo de jugadores requeridos para que el host pueda iniciar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PEV2|Lobby")
	int32 MinPlayersToStart;

	/** Evento disparado en el servidor cuando entra o sale un jugador */
	UPROPERTY(BlueprintAssignable, Category = "PEV2|Lobby")
	FOnLobbyPlayerCountChanged OnLobbyPlayerCountChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PEV2|Lobby")
	int32 ConnectedPlayers;
};
