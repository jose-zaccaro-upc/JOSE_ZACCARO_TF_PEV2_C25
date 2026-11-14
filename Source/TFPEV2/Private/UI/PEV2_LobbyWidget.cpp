// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "UI/PEV2_LobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateColor.h"
#include "Framework/PEV2_LobbyGameMode.h"
#include "Framework/PEV2_LobbyGameState.h"
#include "Core/PEV2_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

void UPEV2_LobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedGameInstance = Cast<UPEV2_GameInstance>(GetGameInstance());

	APlayerController* PC = GetOwningPlayer();
	const bool bIsServer = (PC && PC->HasAuthority()) || (GetWorld() && GetWorld()->GetNetMode() < NM_Client);

	if (Button_StartGame)
	{
		// Solo el Host tiene visible y habilitado el botón de inicio
		Button_StartGame->SetVisibility(bIsServer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Button_StartGame->OnClicked.AddDynamic(this, &UPEV2_LobbyWidget::HandleStartGameClicked);
	}

	if (Button_InviteSteam)
	{
		Button_InviteSteam->OnClicked.AddDynamic(this, &UPEV2_LobbyWidget::HandleInviteSteamClicked);
	}

	if (Button_LeaveLobby)
	{
		Button_LeaveLobby->OnClicked.AddDynamic(this, &UPEV2_LobbyWidget::HandleLeaveLobbyClicked);
	}
	else if (WidgetTree)
	{
		// Auto-inyectar botón "Volver al Menú" si WBP_LobbyMenu no lo incluye explícitamente en UMG
		UPanelWidget* TargetPanel = nullptr;
		if (Button_StartGame && Button_StartGame->GetParent())
		{
			TargetPanel = Button_StartGame->GetParent();
		}
		else if (Button_InviteSteam && Button_InviteSteam->GetParent())
		{
			TargetPanel = Button_InviteSteam->GetParent();
		}
		else if (UPanelWidget* RootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget))
		{
			TargetPanel = RootPanel;
		}

		if (TargetPanel)
		{
			Button_LeaveLobby = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_LeaveLobby_Auto"));
			if (Button_LeaveLobby)
			{
				UTextBlock* BtnText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_LeaveLobby_Auto"));
				if (BtnText)
				{
					BtnText->SetText(FText::FromString(TEXT("VOLVER AL MENÚ")));
					FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle("NormalFont");
					FontInfo.Size = 14;
					BtnText->SetFont(FontInfo);
					BtnText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
					BtnText->SetJustification(ETextJustify::Center);
					Button_LeaveLobby->AddChild(BtnText);
				}

				FButtonStyle Style = Button_LeaveLobby->GetStyle();
				Style.Normal.TintColor = FSlateColor(FLinearColor(0.40f, 0.10f, 0.10f, 0.90f));
				Style.Hovered.TintColor = FSlateColor(FLinearColor(0.70f, 0.15f, 0.15f, 1.0f));
				Style.Pressed.TintColor = FSlateColor(FLinearColor(0.25f, 0.05f, 0.05f, 1.0f));
				Button_LeaveLobby->SetStyle(Style);

				TargetPanel->AddChild(Button_LeaveLobby);
				Button_LeaveLobby->OnClicked.AddDynamic(this, &UPEV2_LobbyWidget::HandleLeaveLobbyClicked);
			}
		}
	}

	if (Text_RoleInfo)
	{
		Text_RoleInfo->SetText(bIsServer ? 
			FText::FromString(TEXT("Rol: HOST (Servidor - Puedes iniciar la partida)")) : 
			FText::FromString(TEXT("Rol: CLIENTE (Esperando que el Host inicie...)")));
	}

	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->OnRoomCodeUpdatedEvent.AddDynamic(this, &UPEV2_LobbyWidget::HandleRoomCodeUpdated);
		UpdateRoomCodeText(CachedGameInstance->GetCurrentRoomCode());
	}

	// Conectar con el GameState replicado (disponible tanto en Server como en Client)
	if (UWorld* World = GetWorld())
	{
		CachedLobbyGameState = World->GetGameState<APEV2_LobbyGameState>();
		if (CachedLobbyGameState.IsValid())
		{
			CachedLobbyGameState->OnLobbyPlayerCountChanged.AddDynamic(this, &UPEV2_LobbyWidget::HandlePlayerCountChanged);
			CachedLobbyGameState->OnLobbyRoomCodeChanged.AddDynamic(this, &UPEV2_LobbyWidget::HandleRoomCodeUpdated);
			UpdatePlayerCountText(CachedLobbyGameState->GetConnectedPlayerCount());
			if (!CachedLobbyGameState->GetRoomCode().IsEmpty())
			{
				UpdateRoomCodeText(CachedLobbyGameState->GetRoomCode());
			}
		}
		else
		{
			// Si el GameState aún está replicándose al cliente, mostrar al menos 1
			UpdatePlayerCountText(1);
		}

		// Timer de sondeo cada 0.5s para vincular el GameState en cuanto llegue la replicación
		World->GetTimerManager().SetTimer(PollTimerHandle, this, &UPEV2_LobbyWidget::PollLobbyState, 0.5f, true);
	}
}

void UPEV2_LobbyWidget::NativeDestruct()
{
	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->OnRoomCodeUpdatedEvent.RemoveDynamic(this, &UPEV2_LobbyWidget::HandleRoomCodeUpdated);
	}

	if (CachedLobbyGameState.IsValid())
	{
		CachedLobbyGameState->OnLobbyPlayerCountChanged.RemoveDynamic(this, &UPEV2_LobbyWidget::HandlePlayerCountChanged);
		CachedLobbyGameState->OnLobbyRoomCodeChanged.RemoveDynamic(this, &UPEV2_LobbyWidget::HandleRoomCodeUpdated);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollTimerHandle);
	}

	Super::NativeDestruct();
}

void UPEV2_LobbyWidget::PollLobbyState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Si no teníamos vinculado el LobbyGameState, intentar obtenerlo
	if (!CachedLobbyGameState.IsValid())
	{
		CachedLobbyGameState = World->GetGameState<APEV2_LobbyGameState>();
		if (CachedLobbyGameState.IsValid())
		{
			CachedLobbyGameState->OnLobbyPlayerCountChanged.AddDynamic(this, &UPEV2_LobbyWidget::HandlePlayerCountChanged);
			CachedLobbyGameState->OnLobbyRoomCodeChanged.AddDynamic(this, &UPEV2_LobbyWidget::HandleRoomCodeUpdated);
			UpdatePlayerCountText(CachedLobbyGameState->GetConnectedPlayerCount());
			if (!CachedLobbyGameState->GetRoomCode().IsEmpty())
			{
				UpdateRoomCodeText(CachedLobbyGameState->GetRoomCode());
			}
			return;
		}
	}

	if (CachedLobbyGameState.IsValid())
	{
		UpdatePlayerCountText(CachedLobbyGameState->GetConnectedPlayerCount());
		if (!CachedLobbyGameState->GetRoomCode().IsEmpty())
		{
			UpdateRoomCodeText(CachedLobbyGameState->GetRoomCode());
		}
	}
	else if (AGameStateBase* BaseGS = World->GetGameState())
	{
		// Fallback utilizando la lista nativa de PlayerState de Unreal Engine
		const int32 Count = FMath::Max(1, BaseGS->PlayerArray.Num());
		UpdatePlayerCountText(Count);
	}
}

void UPEV2_LobbyWidget::HandleStartGameClicked()
{
	APEV2_LobbyGameMode* LobbyGM = Cast<APEV2_LobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (LobbyGM)
	{
		if (Button_StartGame)
		{
			Button_StartGame->SetIsEnabled(false);
		}
		LobbyGM->StartGame();
	}
}

void UPEV2_LobbyWidget::HandleInviteSteamClicked()
{
	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->InviteFriendsViaSteam();
	}
}

void UPEV2_LobbyWidget::HandlePlayerCountChanged(int32 NewPlayerCount)
{
	UpdatePlayerCountText(NewPlayerCount);
}

void UPEV2_LobbyWidget::HandleRoomCodeUpdated(const FString& NewRoomCode)
{
	UpdateRoomCodeText(NewRoomCode);
}

void UPEV2_LobbyWidget::UpdatePlayerCountText(int32 CurrentCount, int32 MaxCount)
{
	if (Text_PlayerCount)
	{
		FText FormattedText = FText::Format(
			NSLOCTEXT("PEV2", "PlayerCountFormat", "Jugadores en el Lobby: {0} / {1}"),
			FText::AsNumber(CurrentCount),
			FText::AsNumber(MaxCount)
		);
		Text_PlayerCount->SetText(FormattedText);
	}
}

void UPEV2_LobbyWidget::UpdateRoomCodeText(const FString& InRoomCode)
{
	if (Text_RoomCode)
	{
		FString DisplayCode = InRoomCode.IsEmpty() ? TEXT("LOCAL/LAN") : InRoomCode;
		Text_RoomCode->SetText(FText::Format(
			NSLOCTEXT("PEV2", "RoomCodeFormat", "CÓDIGO DE SALA: {0}"),
			FText::FromString(DisplayCode)
		));
	}
}

void UPEV2_LobbyWidget::HandleLeaveLobbyClicked()
{
	LeaveLobby();
}

void UPEV2_LobbyWidget::LeaveLobby()
{
	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/PEV2/Maps/L_MainMenu")));
	}
}
