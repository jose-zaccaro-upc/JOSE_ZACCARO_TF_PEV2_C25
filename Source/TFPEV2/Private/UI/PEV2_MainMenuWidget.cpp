// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "UI/PEV2_MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Core/PEV2_GameInstance.h"
#include "OnlineSubsystem.h"

void UPEV2_MainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedGameInstance = Cast<UPEV2_GameInstance>(GetGameInstance());

	if (Button_HostGame)
	{
		Button_HostGame->OnClicked.AddDynamic(this, &UPEV2_MainMenuWidget::HandleHostClicked);
	}

	if (Button_JoinCode)
	{
		Button_JoinCode->OnClicked.AddDynamic(this, &UPEV2_MainMenuWidget::HandleJoinCodeClicked);
	}

	if (Button_JoinIP)
	{
		Button_JoinIP->OnClicked.AddDynamic(this, &UPEV2_MainMenuWidget::HandleJoinIPClicked);
	}

	if (Button_FindLAN)
	{
		Button_FindLAN->OnClicked.AddDynamic(this, &UPEV2_MainMenuWidget::HandleFindLANClicked);
	}

	if (TextBox_IPAddress && TextBox_IPAddress->GetText().IsEmpty())
	{
		TextBox_IPAddress->SetText(FText::FromString(TEXT("127.0.0.1")));
	}

	if (TextBox_RoomCode && TextBox_RoomCode->GetText().IsEmpty())
	{
		TextBox_RoomCode->SetHintText(FText::FromString(TEXT("Código de Sala (ej. AB123)")));
	}

	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->OnCreateSessionCompleteEvent.AddDynamic(this, &UPEV2_MainMenuWidget::OnCreateSessionComplete);
		CachedGameInstance->OnFindSessionsCompleteEvent.AddDynamic(this, &UPEV2_MainMenuWidget::OnFindSessionsComplete);
		CachedGameInstance->OnJoinSessionCompleteEvent.AddDynamic(this, &UPEV2_MainMenuWidget::OnJoinSessionComplete);
		CachedGameInstance->OnNetworkErrorEvent.AddDynamic(this, &UPEV2_MainMenuWidget::OnNetworkErrorOccurred);
	}

	SetStatusMessage(TEXT("Listo para jugar. Crea una sala o ingresa un código."), FLinearColor::White);
}

void UPEV2_MainMenuWidget::NativeDestruct()
{
	if (CachedGameInstance.IsValid())
	{
		CachedGameInstance->OnCreateSessionCompleteEvent.RemoveDynamic(this, &UPEV2_MainMenuWidget::OnCreateSessionComplete);
		CachedGameInstance->OnFindSessionsCompleteEvent.RemoveDynamic(this, &UPEV2_MainMenuWidget::OnFindSessionsComplete);
		CachedGameInstance->OnJoinSessionCompleteEvent.RemoveDynamic(this, &UPEV2_MainMenuWidget::OnJoinSessionComplete);
		CachedGameInstance->OnNetworkErrorEvent.RemoveDynamic(this, &UPEV2_MainMenuWidget::OnNetworkErrorOccurred);
	}

	Super::NativeDestruct();
}

void UPEV2_MainMenuWidget::HandleHostClicked()
{
	if (!CachedGameInstance.IsValid())
	{
		SetStatusMessage(TEXT("Error: GameInstance inválido"), FLinearColor::Red);
		return;
	}

	bool bIsLAN = false;
	if (CheckBox_LAN)
	{
		bIsLAN = CheckBox_LAN->IsChecked();
	}
	else
	{
		// En PIE o si el subsistema es NULL o Steam no está disponible, usar LAN por defecto
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		const bool bIsPIE = GetWorld() ? GetWorld()->IsPlayInEditor() : false;
		const bool bIsNullSub = (OnlineSub == nullptr || OnlineSub->GetSubsystemName() == NULL_SUBSYSTEM);
		bIsLAN = bIsPIE || bIsNullSub;
	}

	SetStatusMessage(FString::Printf(TEXT("Creando partida (%s) y generando código de sala..."),
		bIsLAN ? TEXT("LAN") : TEXT("Online/Steam")), FLinearColor(1.0f, 0.8f, 0.2f));

	if (Button_HostGame) Button_HostGame->SetIsEnabled(false);
	if (Button_JoinCode) Button_JoinCode->SetIsEnabled(false);
	if (Button_JoinIP) Button_JoinIP->SetIsEnabled(false);
	if (Button_FindLAN) Button_FindLAN->SetIsEnabled(false);

	// Hostear partida generando código aleatorio único (Steam / LAN)
	CachedGameInstance->HostSessionWithCode(4, bIsLAN, TEXT(""));
}

void UPEV2_MainMenuWidget::HandleJoinCodeClicked()
{
	if (!CachedGameInstance.IsValid())
	{
		SetStatusMessage(TEXT("Error: GameInstance inválido"), FLinearColor::Red);
		return;
	}

	FString Code = TextBox_RoomCode ? TextBox_RoomCode->GetText().ToString() : TEXT("");
	Code = Code.TrimStartAndEnd().ToUpper();

	if (Code.IsEmpty())
	{
		SetStatusMessage(TEXT("Debes ingresar un código de sala (ej. AB123)."), FLinearColor(1.0f, 0.5f, 0.2f));
		return;
	}

	SetStatusMessage(FString::Printf(TEXT("Buscando sala con código '%s'..."), *Code), FLinearColor(0.2f, 0.8f, 1.0f));

	if (Button_HostGame) Button_HostGame->SetIsEnabled(false);
	if (Button_JoinCode) Button_JoinCode->SetIsEnabled(false);
	if (Button_JoinIP) Button_JoinIP->SetIsEnabled(false);
	if (Button_FindLAN) Button_FindLAN->SetIsEnabled(false);

	bPendingFindLAN = false;
	CachedGameInstance->JoinSessionByCode(Code);
}

void UPEV2_MainMenuWidget::HandleJoinIPClicked()
{
	if (!CachedGameInstance.IsValid())
	{
		SetStatusMessage(TEXT("Error: GameInstance inválido"), FLinearColor::Red);
		return;
	}

	FString TargetIP = TextBox_IPAddress ? TextBox_IPAddress->GetText().ToString() : TEXT("127.0.0.1");
	TargetIP = TargetIP.TrimStartAndEnd();

	if (TargetIP.IsEmpty())
	{
		TargetIP = TEXT("127.0.0.1");
	}

	SetStatusMessage(FString::Printf(TEXT("Conectando a IP: %s..."), *TargetIP), FLinearColor(0.2f, 0.8f, 1.0f));

	CachedGameInstance->JoinSessionByIP(TargetIP);
}

void UPEV2_MainMenuWidget::HandleFindLANClicked()
{
	if (!CachedGameInstance.IsValid())
	{
		SetStatusMessage(TEXT("Error: GameInstance inválido"), FLinearColor::Red);
		return;
	}

	SetStatusMessage(TEXT("Buscando sesiones en la red local (LAN)..."), FLinearColor(1.0f, 0.8f, 0.2f));

	if (Button_FindLAN) Button_FindLAN->SetIsEnabled(false);
	if (Button_HostGame) Button_HostGame->SetIsEnabled(false);
	if (Button_JoinCode) Button_JoinCode->SetIsEnabled(false);
	if (Button_JoinIP) Button_JoinIP->SetIsEnabled(false);

	bPendingFindLAN = true;
	CachedGameInstance->FindSessions(true);
}

void UPEV2_MainMenuWidget::OnCreateSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		SetStatusMessage(TEXT("Sesión creada exitosamente. Viajando al Lobby..."), FLinearColor::Green);
	}
	else
	{
		SetStatusMessage(TEXT("Error al crear sesión en el servidor"), FLinearColor::Red);
		if (Button_HostGame) Button_HostGame->SetIsEnabled(true);
		if (Button_JoinCode) Button_JoinCode->SetIsEnabled(true);
		if (Button_JoinIP) Button_JoinIP->SetIsEnabled(true);
		if (Button_FindLAN) Button_FindLAN->SetIsEnabled(true);
	}
}

void UPEV2_MainMenuWidget::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (Button_FindLAN) Button_FindLAN->SetIsEnabled(true);
	if (Button_HostGame) Button_HostGame->SetIsEnabled(true);
	if (Button_JoinCode) Button_JoinCode->SetIsEnabled(true);
	if (Button_JoinIP) Button_JoinIP->SetIsEnabled(true);

	if (!bWasSuccessful || !CachedGameInstance.IsValid())
	{
		bPendingFindLAN = false;
		SetStatusMessage(TEXT("Error en la búsqueda de partidas."), FLinearColor::Red);
		return;
	}

	int32 Count = CachedGameInstance->GetSearchResultsCount();
	if (Count > 0)
	{
		if (bPendingFindLAN)
		{
			bPendingFindLAN = false;
			SetStatusMessage(FString::Printf(TEXT("Partida LAN encontrada (%d disponible/s). Conectando..."), Count), FLinearColor::Green);
			CachedGameInstance->JoinFoundSession(0);
		}
		else
		{
			SetStatusMessage(FString::Printf(TEXT("Se encontraron %d partidas."), Count), FLinearColor::Green);
		}
	}
	else
	{
		if (bPendingFindLAN && GetWorld() && GetWorld()->IsPlayInEditor())
		{
			// En Play In Editor (PIE), el broadcast UDP local de beacons suele estar bloqueado por ejecutarse en el mismo proceso.
			// Conectar automáticamente al servidor local activo de PIE en 127.0.0.1
			bPendingFindLAN = false;
			SetStatusMessage(TEXT("Servidor local (PIE) detectado. Conectando a 127.0.0.1..."), FLinearColor::Green);
			CachedGameInstance->JoinSessionByIP(TEXT("127.0.0.1"));
			return;
		}

		bPendingFindLAN = false;
		SetStatusMessage(TEXT("No se encontraron sesiones LAN activas. Verifica que el Host haya iniciado la sala."), FLinearColor(1.0f, 0.6f, 0.2f));
	}
}

void UPEV2_MainMenuWidget::OnJoinSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		SetStatusMessage(TEXT("Conectado con éxito. Cargando nivel..."), FLinearColor::Green);
	}
	else
	{
		SetStatusMessage(TEXT("No se pudo conectar a la partida."), FLinearColor::Red);
		if (Button_HostGame) Button_HostGame->SetIsEnabled(true);
		if (Button_JoinCode) Button_JoinCode->SetIsEnabled(true);
		if (Button_JoinIP) Button_JoinIP->SetIsEnabled(true);
		if (Button_FindLAN) Button_FindLAN->SetIsEnabled(true);
	}
}

void UPEV2_MainMenuWidget::OnNetworkErrorOccurred(const FString& ErrorMessage)
{
	SetStatusMessage(ErrorMessage, FLinearColor::Red);

	if (Button_HostGame) Button_HostGame->SetIsEnabled(true);
	if (Button_JoinCode) Button_JoinCode->SetIsEnabled(true);
	if (Button_JoinIP) Button_JoinIP->SetIsEnabled(true);
	if (Button_FindLAN) Button_FindLAN->SetIsEnabled(true);
}

void UPEV2_MainMenuWidget::SetStatusMessage(const FString& InMessage, const FLinearColor& InColor)
{
	if (Text_StatusInfo)
	{
		Text_StatusInfo->SetText(FText::FromString(InMessage));
		Text_StatusInfo->SetColorAndOpacity(FSlateColor(InColor));
	}
}
