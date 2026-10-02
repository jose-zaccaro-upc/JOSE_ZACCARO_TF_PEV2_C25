// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PEV2_MenuPlayerController.generated.h"

class UUserWidget;

/**
 * PlayerController especializado para menús (L_MainMenu).
 * Instancia automáticamente el widget del menú y configura cursor e inputs.
 */
UCLASS()
class TFPEV2_API APEV2_MenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APEV2_MenuPlayerController();

protected:
	virtual void BeginPlay() override;

public:
	/** Clase del Widget de Menú Principal (por ejemplo WBP_MainMenu) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PEV2|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveMenuWidget;
};
