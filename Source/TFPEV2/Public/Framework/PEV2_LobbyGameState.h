// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PEV2_LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerCountReplicated, int32, NewPlayerCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyRoomCodeReplicated, const FString&, NewRoomCode);

/**
 * GameState para la sala de espera (L_Lobby).
 * Replicado automáticamente por el servidor a todos los clientes para mantener sincronizado
 * el número exacto de jugadores en el lobby y el estado de la sala.
 */
UCLASS()
class TFPEV2_API APEV2_LobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APEV2_LobbyGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Cantidad de jugadores actualmente conectados en el Lobby (Sincronizado a todos) */
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayerCount, BlueprintReadOnly, Category = "PEV2|Lobby")
	int32 ConnectedPlayerCount;

	UFUNCTION()
	void OnRep_ConnectedPlayerCount();

	/** Código de sala activo (Sincronizado a todos los clientes) */
	UPROPERTY(ReplicatedUsing = OnRep_RoomCode, BlueprintReadOnly, Category = "PEV2|Lobby")
	FString RoomCode;

	UFUNCTION()
	void OnRep_RoomCode();

	/** Evento disparado en Server y Clientes al actualizarse el número de jugadores */
	UPROPERTY(BlueprintAssignable, Category = "PEV2|Lobby")
	FOnLobbyPlayerCountReplicated OnLobbyPlayerCountChanged;

	/** Evento disparado en Clientes al replicarse el código de sala */
	UPROPERTY(BlueprintAssignable, Category = "PEV2|Lobby")
	FOnLobbyRoomCodeReplicated OnLobbyRoomCodeChanged;

	/** Actualiza el contador de jugadores (Solo Servidor con autoridad) */
	void SetConnectedPlayerCount(int32 NewCount);

	/** Asigna el código de sala para sincronizar a clientes (Solo Servidor) */
	void SetRoomCode(const FString& NewCode);

	UFUNCTION(BlueprintPure, Category = "PEV2|Lobby")
	int32 GetConnectedPlayerCount() const { return ConnectedPlayerCount; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Lobby")
	FString GetRoomCode() const { return RoomCode; }
};
