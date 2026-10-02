// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PEV2_LobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UPEV2_GameInstance;
class APEV2_LobbyGameState;

/**
 * Widget UMG para la pantalla del Lobby.
 * Muestra el código de sala aleatorio ("AB123"), botón de invitaciones de Steam y jugadores conectados sincronizados en red.
 */
UCLASS()
class TFPEV2_API UPEV2_LobbyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_StartGame;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_InviteSteam;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RoomCode;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_PlayerCount;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RoleInfo;

	UFUNCTION()
	void HandleStartGameClicked();

	UFUNCTION()
	void HandleInviteSteamClicked();

	UFUNCTION()
	void HandlePlayerCountChanged(int32 NewPlayerCount);

	UFUNCTION()
	void HandleRoomCodeUpdated(const FString& NewRoomCode);

	/** Polling ligero cada 0.5s para garantizar sincronización perfecta en clientes */
	void PollLobbyState();

public:
	/** Actualiza el contador de jugadores en pantalla */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Lobby")
	void UpdatePlayerCountText(int32 CurrentCount, int32 MaxCount = 4);

	/** Actualiza el texto visual del código de sala */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Lobby")
	void UpdateRoomCodeText(const FString& InRoomCode);

private:
	TWeakObjectPtr<UPEV2_GameInstance> CachedGameInstance;
	TWeakObjectPtr<APEV2_LobbyGameState> CachedLobbyGameState;
	FTimerHandle PollTimerHandle;
};
