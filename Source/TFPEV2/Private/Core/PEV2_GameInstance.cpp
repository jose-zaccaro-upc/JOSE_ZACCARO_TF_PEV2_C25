// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Core/PEV2_GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

const FName UPEV2_GameInstance::SESSION_NAME = FName(TEXT("PEV2_GameSession"));
const FName UPEV2_GameInstance::SETTING_ROOM_CODE = FName(TEXT("ROOM_CODE"));

UPEV2_GameInstance::UPEV2_GameInstance()
{
	LobbyMapPath = TEXT("/Game/PEV2/Maps/L_Lobby");
	GameArenaMapPath = TEXT("/Game/PEV2/Maps/L_GameArena");
	MainMenuMapPath = TEXT("/Game/PEV2/Maps/L_MainMenu");

	bPendingHost = false;
	PendingMaxPlayers = 4;
	bPendingIsLAN = false;
	PendingCustomCode = TEXT("");

	bPendingJoinByCode = false;
	PendingRoomCodeToJoin = TEXT("");
	bJoinByCodeLANAttempted = false;
	bJoinByCodeOnlineAttempted = false;
	CurrentRoomCode = TEXT("");

	bPendingReturnToMainMenu = false;
	bPendingJoinSession = false;
	PendingJoinSessionIndex = INDEX_NONE;
}

IOnlineSessionPtr UPEV2_GameInstance::GetSessionInterface() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		IOnlineSessionPtr WorldSessionInterface = Online::GetSessionInterface(World);
		if (WorldSessionInterface.IsValid())
		{
			return WorldSessionInterface;
		}
	}

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	return OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
}

void UPEV2_GameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] OnlineSubsystem cargado con éxito: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		// Suscribirse a invitaciones entrantes de amigos (ej. Steam chat / menú de amigos)
		SessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UPEV2_GameInstance::OnSessionUserInviteAccepted)
		);
	}

	// Manejo robusto de errores de red y viajes para prevenir crasheos
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UPEV2_GameInstance::HandleNetworkFailure);
		GEngine->OnTravelFailure().AddUObject(this, &UPEV2_GameInstance::HandleTravelFailure);
	}
}

void UPEV2_GameInstance::Shutdown()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	Super::Shutdown();
}

FString UPEV2_GameInstance::GenerateRoomCode(int32 Length)
{
	// Alfabeto alfanumérico sin caracteres ambiguos (excluye 0, O, 1, I)
	static const TCHAR CharSet[] = TEXT("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");
	const int32 CharSetLen = UE_ARRAY_COUNT(CharSet) - 1;

	FString Code;
	Code.Reserve(Length);

	for (int32 i = 0; i < Length; ++i)
	{
		const int32 RandomIndex = FMath::RandRange(0, CharSetLen - 1);
		Code.AppendChar(CharSet[RandomIndex]);
	}

	return Code;
}

void UPEV2_GameInstance::HostSessionWithCode(int32 MaxPlayers, bool bIsLAN, const FString& CustomCode)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] Error al Hostear: SessionInterface inválido."));
		OnNetworkErrorEvent.Broadcast(TEXT("Error: No se pudo conectar al subsistema de red."));
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	// Si ya existe una sesión previa, destruirla antes de recrear una nueva
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Sesión existente detectada. Destruyendo antes de recrear..."));
		bPendingHost = true;
		PendingMaxPlayers = MaxPlayers;
		bPendingIsLAN = bIsLAN;
		PendingCustomCode = CustomCode;

		if (ExistingSession->SessionState != EOnlineSessionState::Destroying)
		{
			DestroyCurrentSession();
		}
		return;
	}

	// Asignar o generar código de sala único
	CurrentRoomCode = CustomCode.TrimStartAndEnd().ToUpper();
	if (CurrentRoomCode.IsEmpty())
	{
		CurrentRoomCode = GenerateRoomCode(5);
	}
	OnRoomCodeUpdatedEvent.Broadcast(CurrentRoomCode);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = bIsLAN;
	LastSessionSettings->NumPublicConnections = FMath::Clamp(MaxPlayers, 1, 16);
	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = !bIsLAN;
	LastSessionSettings->bUsesPresence = !bIsLAN;
	LastSessionSettings->bUseLobbiesIfAvailable = !bIsLAN;
	LastSessionSettings->bAllowInvites = !bIsLAN;

	// Clave de búsqueda personalizada para que los clientes ubiquen la partida mediante este código
	LastSessionSettings->Set(SETTING_ROOM_CODE, CurrentRoomCode, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(SETTING_MAPNAME, LobbyMapPath, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UPEV2_GameInstance::OnCreateSessionComplete)
	);

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const int32 ControllerId = LocalPlayer ? LocalPlayer->GetControllerId() : 0;

	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Creando sesión Listen Server. Código: '%s' | Jugadores: %d (LAN: %s)..."),
		*CurrentRoomCode, LastSessionSettings->NumPublicConnections, bIsLAN ? TEXT("True") : TEXT("False"));

	if (!SessionInterface->CreateSession(ControllerId, SESSION_NAME, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] Falló la llamada a CreateSession"));
		OnNetworkErrorEvent.Broadcast(TEXT("Error al iniciar la sala en el servidor."));
		OnCreateSessionCompleteEvent.Broadcast(false);
	}
}

void UPEV2_GameInstance::HostSession(int32 MaxPlayers, bool bIsLAN)
{
	HostSessionWithCode(MaxPlayers, bIsLAN, TEXT(""));
}

void UPEV2_GameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] OnCreateSessionComplete: %s (Éxito: %s) | Código: %s"),
		*SessionName.ToString(), bWasSuccessful ? TEXT("True") : TEXT("False"), *CurrentRoomCode);

	OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);

	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// Abrir nivel de Lobby como Listen Server para que otros clientes puedan unirse
			FString TravelURL = FString::Printf(TEXT("%s?listen"), *LobbyMapPath);
			UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Host viajando al Lobby: %s"), *TravelURL);
			UGameplayStatics::OpenLevel(World, FName(*LobbyMapPath), true, TEXT("listen"));
		}
	}
	else
	{
		OnNetworkErrorEvent.Broadcast(TEXT("No se pudo crear la sesión de juego."));
	}
}

void UPEV2_GameInstance::FindSessions(bool bIsLAN)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] Error al buscar: SessionInterface inválido."));
		OnNetworkErrorEvent.Broadcast(TEXT("Subsistema de red no disponible para buscar partidas."));
		OnFindSessionsCompleteEvent.Broadcast(false);
		return;
	}

	// Si había una búsqueda anterior en progreso, cancelarla
	if (LastSessionSearch.IsValid() && LastSessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		SessionInterface->CancelFindSessions();
	}

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->bIsLanQuery = bIsLAN;
	LastSessionSearch->MaxSearchResults = 50;
	LastSessionSearch->PingBucketSize = 50;

	// Si no es LAN, buscar sesiones en Steam Lobbies
	if (!bIsLAN)
	{
		LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UPEV2_GameInstance::OnFindSessionsComplete)
	);

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const int32 ControllerId = LocalPlayer ? LocalPlayer->GetControllerId() : 0;

	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Iniciando búsqueda de sesiones (LAN: %s)..."), bIsLAN ? TEXT("True") : TEXT("False"));

	if (!SessionInterface->FindSessions(ControllerId, LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		UE_LOG(LogTemp, Warning, TEXT("[PEV2_GameInstance] Falló la llamada a FindSessions"));
		OnNetworkErrorEvent.Broadcast(TEXT("Falló la petición de búsqueda de partidas."));
		OnFindSessionsCompleteEvent.Broadcast(false);
	}
}

void UPEV2_GameInstance::JoinSessionByCode(const FString& InRoomCode)
{
	FString CleanCode = InRoomCode.TrimStartAndEnd().ToUpper();
	if (CleanCode.IsEmpty())
	{
		OnNetworkErrorEvent.Broadcast(TEXT("Debes ingresar un código de sala válido."));
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	bPendingJoinByCode = true;
	PendingRoomCodeToJoin = CleanCode;
	bJoinByCodeLANAttempted = false;
	bJoinByCodeOnlineAttempted = false;

	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Buscando salas para unirse por código: %s"), *PendingRoomCodeToJoin);

	// Determinar si iniciamos por LAN o Online según el entorno
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	const bool bIsPIE = GetWorld() ? GetWorld()->IsPlayInEditor() : false;
	const bool bIsNullSub = (OnlineSub == nullptr || OnlineSub->GetSubsystemName() == NULL_SUBSYSTEM);
	const bool bStartLAN = bIsPIE || bIsNullSub;

	if (bStartLAN)
	{
		bJoinByCodeLANAttempted = true;
		FindSessions(true);
	}
	else
	{
		bJoinByCodeOnlineAttempted = true;
		FindSessions(false);
	}
}

void UPEV2_GameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	const int32 ResultsCount = (bWasSuccessful && LastSessionSearch.IsValid()) ? LastSessionSearch->SearchResults.Num() : 0;
	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] OnFindSessionsComplete. Éxito: %s | Sesiones encontradas: %d"),
		bWasSuccessful ? TEXT("True") : TEXT("False"), ResultsCount);

	// Si veníamos de una petición de unirse por código, filtrar los resultados
	if (bPendingJoinByCode)
	{
		int32 FoundMatchIndex = INDEX_NONE;

		if (bWasSuccessful && LastSessionSearch.IsValid())
		{
			for (int32 i = 0; i < LastSessionSearch->SearchResults.Num(); ++i)
			{
				const FOnlineSessionSearchResult& SearchResult = LastSessionSearch->SearchResults[i];
				FString SessionRoomCode;

				if (SearchResult.Session.SessionSettings.Get(SETTING_ROOM_CODE, SessionRoomCode))
				{
					if (SessionRoomCode.Equals(PendingRoomCodeToJoin, ESearchCase::IgnoreCase))
					{
						FoundMatchIndex = i;
						break;
					}
				}
			}
		}

		if (FoundMatchIndex != INDEX_NONE)
		{
			bPendingJoinByCode = false;
			UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Sala '%s' localizada en índice %d. Conectando..."), *PendingRoomCodeToJoin, FoundMatchIndex);
			CurrentRoomCode = PendingRoomCodeToJoin;
			OnRoomCodeUpdatedEvent.Broadcast(CurrentRoomCode);
			OnFindSessionsCompleteEvent.Broadcast(true);
			JoinFoundSession(FoundMatchIndex);
			return;
		}

		// En Play In Editor (PIE), el broadcast UDP LAN local dentro del mismo proceso suele no retornar paquetes.
		// Conectar automáticamente al host local activo en 127.0.0.1 con el código de sala establecido.
		if (GetWorld() && GetWorld()->IsPlayInEditor())
		{
			bPendingJoinByCode = false;
			UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] PIE detectado: Conectando directamente al Host local en 127.0.0.1 con el código '%s'..."), *PendingRoomCodeToJoin);
			CurrentRoomCode = PendingRoomCodeToJoin;
			OnRoomCodeUpdatedEvent.Broadcast(CurrentRoomCode);
			OnFindSessionsCompleteEvent.Broadcast(true);
			JoinSessionByIP(TEXT("127.0.0.1"));
			return;
		}

		// Si no se encontró en esta búsqueda, probar la alternativa (LAN vs Online)
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		const bool bCanUseSteam = (OnlineSub && OnlineSub->GetSubsystemName() != NULL_SUBSYSTEM);

		if (!bJoinByCodeLANAttempted)
		{
			UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Código '%s' no encontrado en servicio online. Buscando en red local (LAN)..."), *PendingRoomCodeToJoin);
			bJoinByCodeLANAttempted = true;
			FindSessions(true);
			return;
		}
		else if (!bJoinByCodeOnlineAttempted && bCanUseSteam)
		{
			UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Código '%s' no encontrado en LAN. Buscando en servicio online..."), *PendingRoomCodeToJoin);
			bJoinByCodeOnlineAttempted = true;
			FindSessions(false);
			return;
		}

		// Si ambas vías fueron intentadas y no se halló coincidencia
		bPendingJoinByCode = false;
		UE_LOG(LogTemp, Warning, TEXT("[PEV2_GameInstance] Ninguna sala activa coincidió con el código: %s"), *PendingRoomCodeToJoin);
		OnNetworkErrorEvent.Broadcast(FString::Printf(TEXT("Código '%s' incorrecto o la partida ya no está disponible."), *PendingRoomCodeToJoin));
		OnFindSessionsCompleteEvent.Broadcast(false);
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	OnFindSessionsCompleteEvent.Broadcast(bWasSuccessful);
}

void UPEV2_GameInstance::JoinFoundSession(int32 SessionIndex)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] Error al unirse: SessionInterface inválido."));
		OnNetworkErrorEvent.Broadcast(TEXT("Subsistema de red no disponible para unirse."));
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (!LastSessionSearch.IsValid() || !LastSessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PEV2_GameInstance] Índice de sesión no válido: %d"), SessionIndex);
		OnNetworkErrorEvent.Broadcast(TEXT("La sala seleccionada ya no existe."));
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	// Si el cliente aún tiene una sesión previa abierta, destruirla antes de unirse
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Destruyendo sesión previa antes de unirse a la nueva..."));
		bPendingJoinSession = true;
		PendingJoinSessionIndex = SessionIndex;
		DestroyCurrentSession();
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPEV2_GameInstance::OnJoinSessionComplete)
	);

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const int32 ControllerId = LocalPlayer ? LocalPlayer->GetControllerId() : 0;

	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Intentando unirse a sesión índice: %d"), SessionIndex);

	if (!SessionInterface->JoinSession(ControllerId, SESSION_NAME, LastSessionSearch->SearchResults[SessionIndex]))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] Falló la llamada a JoinSession"));
		OnNetworkErrorEvent.Broadcast(TEXT("Error al iniciar conexión con la sala."));
		OnJoinSessionCompleteEvent.Broadcast(false);
	}
}

void UPEV2_GameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	const bool bWasSuccessful = (Result == EOnJoinSessionCompleteResult::Success);
	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] OnJoinSessionComplete: %s (Resultado: %d)"), *SessionName.ToString(), (int32)Result);

	OnJoinSessionCompleteEvent.Broadcast(bWasSuccessful);

	if (bWasSuccessful && SessionInterface.IsValid())
	{
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
		{
			APlayerController* PlayerController = GetFirstLocalPlayerController();
			if (PlayerController)
			{
				UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Viajando a servidor resuelto: %s"), *ConnectInfo);
				PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
			}
			else
			{
				OnNetworkErrorEvent.Broadcast(TEXT("Error interno: Controlador de jugador local no encontrado."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] No se pudo obtener la cadena de conexión de la sesión."));
			OnNetworkErrorEvent.Broadcast(TEXT("No se pudo obtener la dirección de conexión del servidor."));
		}
	}
	else
	{
		FString ErrorMsg = TEXT("No fue posible unirse a la sala.");
		if (Result == EOnJoinSessionCompleteResult::SessionIsFull)
		{
			ErrorMsg = TEXT("La sala se encuentra llena.");
		}
		else if (Result == EOnJoinSessionCompleteResult::SessionDoesNotExist)
		{
			ErrorMsg = TEXT("La sala ha dejado de existir.");
		}
		OnNetworkErrorEvent.Broadcast(ErrorMsg);
	}
}

void UPEV2_GameInstance::JoinSessionByIP(const FString& InIPAddress)
{
	FString TargetAddress = InIPAddress.TrimStartAndEnd();
	if (TargetAddress.IsEmpty())
	{
		TargetAddress = TEXT("127.0.0.1");
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController)
	{
		UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Conexión directa por IP a: %s"), *TargetAddress);
		PlayerController->ClientTravel(TargetAddress, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] PlayerController local no disponible para ClientTravel"));
		OnNetworkErrorEvent.Broadcast(TEXT("Error: No se pudo iniciar el viaje directo por IP."));
	}
}

void UPEV2_GameInstance::InviteFriendsViaSteam()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		OnNetworkErrorEvent.Broadcast(TEXT("Steam no está activo en este momento."));
		return;
	}

	IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
		const int32 ControllerId = LocalPlayer ? LocalPlayer->GetControllerId() : 0;

		UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Abriendo interfaz de invitación de Steam..."));
		ExternalUI->ShowInviteUI(ControllerId, SESSION_NAME);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PEV2_GameInstance] ExternalUI no soportado por el subsistema actual: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
		OnNetworkErrorEvent.Broadcast(TEXT("La interfaz de invitación directa no está disponible en este subsistema."));
	}
}

void UPEV2_GameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Invitación de Steam recibida y aceptada. Éxito: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!bWasSuccessful || !SessionInterface.IsValid())
	{
		OnNetworkErrorEvent.Broadcast(TEXT("No se pudo procesar la invitación recibida de Steam."));
		return;
	}

	// Extraer código de sala de la invitación si está disponible
	FString InviteRoomCode;
	if (InviteResult.Session.SessionSettings.Get(SETTING_ROOM_CODE, InviteRoomCode))
	{
		CurrentRoomCode = InviteRoomCode;
		OnRoomCodeUpdatedEvent.Broadcast(CurrentRoomCode);
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPEV2_GameInstance::OnJoinSessionComplete)
	);

	if (!SessionInterface->JoinSession(ControllerId, SESSION_NAME, InviteResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnNetworkErrorEvent.Broadcast(TEXT("Error al conectarse a la partida invitada."));
	}
}

void UPEV2_GameInstance::DestroyCurrentSession()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPEV2_GameInstance::OnDestroySessionComplete)
	);

	if (!SessionInterface->DestroySession(SESSION_NAME))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnDestroySessionComplete(SESSION_NAME, false);
	}
}

void UPEV2_GameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Sesión destruida: %s (Éxito: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("True") : TEXT("False"));

	if (bPendingReturnToMainMenu)
	{
		bPendingReturnToMainMenu = false;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ReturnToMainMenuTimerHandle);
			UGameplayStatics::OpenLevel(World, FName(*MainMenuMapPath));
		}
		return;
	}

	if (bPendingHost)
	{
		bPendingHost = false;
		HostSessionWithCode(PendingMaxPlayers, bPendingIsLAN, PendingCustomCode);
		return;
	}

	if (bPendingJoinSession)
	{
		bPendingJoinSession = false;
		JoinFoundSession(PendingJoinSessionIndex);
		return;
	}
}

void UPEV2_GameInstance::LeaveSessionAndReturnToMainMenu()
{
	UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] LeaveSessionAndReturnToMainMenu solicitado."));

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	FNamedOnlineSession* ExistingSession = SessionInterface.IsValid() ? SessionInterface->GetNamedSession(SESSION_NAME) : nullptr;

	if (ExistingSession != nullptr)
	{
		bPendingReturnToMainMenu = true;
		DestroyCurrentSession();

		// Timeout de seguridad de 1.2s para forzar el viaje si el backend no responde
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ReturnToMainMenuTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (bPendingReturnToMainMenu)
				{
					bPendingReturnToMainMenu = false;
					UE_LOG(LogTemp, Warning, TEXT("[PEV2_GameInstance] Timeout esperando destrucción de sesión. Viajando forzosamente a MainMenu..."));
					if (UWorld* CurrentWorld = GetWorld())
					{
						UGameplayStatics::OpenLevel(CurrentWorld, FName(*MainMenuMapPath));
					}
				}
			}), 1.2f, false);
		}
	}
	else
	{
		bPendingReturnToMainMenu = false;
		if (UWorld* World = GetWorld())
		{
			UGameplayStatics::OpenLevel(World, FName(*MainMenuMapPath));
		}
	}
}

void UPEV2_GameInstance::CleanupStaleSessions()
{
	bPendingJoinByCode = false;
	bJoinByCodeLANAttempted = false;
	bJoinByCodeOnlineAttempted = false;
	bPendingHost = false;
	bPendingJoinSession = false;
	bPendingReturnToMainMenu = false;

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		// Cancelar cualquier búsqueda previa que haya quedado pendiente
		if (LastSessionSearch.IsValid() && LastSessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
		{
			SessionInterface->CancelFindSessions();
		}

		// Si quedó una sesión residual en memoria, destruirla
		FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("[PEV2_GameInstance] Limpiando sesión residual en menú principal: %s"), *SESSION_NAME.ToString());
			SessionInterface->DestroySession(SESSION_NAME);
		}
	}
}

int32 UPEV2_GameInstance::GetSearchResultsCount() const
{
	return (LastSessionSearch.IsValid()) ? LastSessionSearch->SearchResults.Num() : 0;
}

void UPEV2_GameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] HandleNetworkFailure: %s (Tipo: %d)"), *ErrorString, (int32)FailureType);

	FString UserFriendlyError = FString::Printf(TEXT("Fallo de red: %s"), *ErrorString);
	if (FailureType == ENetworkFailure::ConnectionLost || FailureType == ENetworkFailure::ConnectionTimeout)
	{
		UserFriendlyError = TEXT("Se ha perdido la conexión con el servidor de la partida.");
	}

	OnNetworkErrorEvent.Broadcast(UserFriendlyError);

	// Regresar de forma segura al menú principal sin crashear
	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && !MainMenuMapPath.IsEmpty())
	{
		UGameplayStatics::OpenLevel(CurrentWorld, FName(*MainMenuMapPath));
	}
}

void UPEV2_GameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("[PEV2_GameInstance] HandleTravelFailure: %s (Tipo: %d)"), *ErrorString, (int32)FailureType);

	FString UserFriendlyError = FString::Printf(TEXT("Error al cargar nivel: %s"), *ErrorString);
	if (FailureType == ETravelFailure::ServerTravelFailure || FailureType == ETravelFailure::ClientTravelFailure)
	{
		UserFriendlyError = TEXT("No se pudo completar el viaje al mapa de juego.");
	}

	OnNetworkErrorEvent.Broadcast(UserFriendlyError);

	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && !MainMenuMapPath.IsEmpty())
	{
		UGameplayStatics::OpenLevel(CurrentWorld, FName(*MainMenuMapPath));
	}
}
