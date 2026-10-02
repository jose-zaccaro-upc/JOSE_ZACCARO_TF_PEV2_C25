// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PEV2_LobbyPlayerController.generated.h"

class UUserWidget;

/**
 * PlayerController para la sala de espera (L_Lobby).
 * Instancia automáticamente WBP_LobbyMenu para todos los clientes y el host.
 */
UCLASS()
class TFPEV2_API APEV2_LobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APEV2_LobbyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void ReceivedPlayer() override;

public:
	/** Crea e inicializa el widget del lobby en la pantalla local si aún no existe */
	UFUNCTION(BlueprintCallable, Category = "PEV2|UI")
	void SetupLobbyUI();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PEV2|UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveLobbyWidget;
};
