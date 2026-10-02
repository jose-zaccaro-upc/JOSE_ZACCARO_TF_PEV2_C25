// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PEV2_MainMenuWidget.generated.h"

class UButton;
class UCheckBox;
class UEditableTextBox;
class UTextBlock;
class UPEV2_GameInstance;

/**
 * Clase base C++ para WBP_MainMenu.
 * Vincula los botones de Host (con generación de código), Unión por Código ("AB123"), IP directa y Steam.
 */
UCLASS()
class TFPEV2_API UPEV2_MainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	// ==========================================
	// COMPONENTES UMG (BindWidgetOptional)
	// ==========================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_HostGame;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_JoinCode;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_JoinIP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_FindLAN;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> CheckBox_LAN;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> TextBox_RoomCode;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> TextBox_IPAddress;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_StatusInfo;

	// ==========================================
	// ACCIONES DE BOTONES
	// ==========================================

	UFUNCTION()
	void HandleHostClicked();

	UFUNCTION()
	void HandleJoinCodeClicked();

	UFUNCTION()
	void HandleJoinIPClicked();

	UFUNCTION()
	void HandleFindLANClicked();

	// ==========================================
	// CALLBACKS DE RED DESDE GAMEINSTANCE
	// ==========================================

	UFUNCTION()
	void OnCreateSessionComplete(bool bWasSuccessful);

	UFUNCTION()
	void OnFindSessionsComplete(bool bWasSuccessful);

	UFUNCTION()
	void OnJoinSessionComplete(bool bWasSuccessful);

	UFUNCTION()
	void OnNetworkErrorOccurred(const FString& ErrorMessage);

	/** Actualiza el texto de estado en pantalla */
	void SetStatusMessage(const FString& InMessage, const FLinearColor& InColor = FLinearColor::White);

private:
	TWeakObjectPtr<UPEV2_GameInstance> CachedGameInstance;
	bool bPendingFindLAN = false;
};
