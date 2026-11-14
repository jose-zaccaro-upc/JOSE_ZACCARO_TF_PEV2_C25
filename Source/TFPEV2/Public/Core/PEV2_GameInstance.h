// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "PEV2_GameInstance.generated.h"

// Delegados dinámicos para notificar a la interfaz de usuario (UMG)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPEV2OnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPEV2OnFindSessionsComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPEV2OnJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPEV2OnNetworkErrorOccurred, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPEV2OnRoomCodeUpdated, const FString&, NewRoomCode);

/**
 * GameInstance central para la gestión de sesiones de red en Listen Server, Steam y LAN.
 */
UCLASS()
class TFPEV2_API UPEV2_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPEV2_GameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	/* Nombre por defecto de la sesión */
	static const FName SESSION_NAME;

	/* Clave de configuración para el código de sala en la sesión */
	static const FName SETTING_ROOM_CODE;

	// ==========================================
	// FUNCIONES EXPUESTAS A BLUEPRINTS
	// ==========================================

	/**
	 * Genera un código de sala aleatorio e irrepetible (ej. "AB123").
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	FString GenerateRoomCode(int32 Length = 5);

	/**
	 * Crea y hostea una nueva sesión como Listen Server asignándole un código de sala.
	 * @param MaxPlayers Número máximo de jugadores permitidos (1 a 4).
	 * @param bIsLAN Define si la búsqueda/creación es por LAN (false para Steam/Internet).
	 * @param CustomCode Código personalizado opcional. Si está vacío se genera uno automáticamente.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void HostSessionWithCode(int32 MaxPlayers = 4, bool bIsLAN = false, const FString& CustomCode = TEXT(""));

	/**
	 * Crea y hostea una nueva sesión (compatibilidad hacia atrás).
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void HostSession(int32 MaxPlayers = 4, bool bIsLAN = true);

	/**
	 * Busca sesiones disponibles en el subsistema activo.
	 * @param bIsLAN Define si se buscan partidas en LAN.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void FindSessions(bool bIsLAN = false);

	/**
	 * Busca y se une directamente a una sala utilizando su código aleatorio (ej. "AB123").
	 * @param InRoomCode Código de sala al que unirse.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void JoinSessionByCode(const FString& InRoomCode);

	/**
	 * Se une a una sesión encontrada por índice en la lista de resultados.
	 * @param SessionIndex Índice de la sesión a la cual unirse.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void JoinFoundSession(int32 SessionIndex = 0);

	/**
	 * Conexión directa a través de dirección IP (ej. "127.0.0.1" o IP LAN).
	 * @param InIPAddress Dirección IP y puerto opcional (ej: "192.168.1.50:7777").
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void JoinSessionByIP(const FString& InIPAddress);

	/**
	 * Abre la interfaz de Steam Overlay para invitar amigos directamente a la sesión.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void InviteFriendsViaSteam();

	/**
	 * Destruye la sesión activa actual de forma segura.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void DestroyCurrentSession();

	/**
	 * Abandona la sesión actual, destruyéndola limpiamente en el backend antes de viajar a L_MainMenu.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void LeaveSessionAndReturnToMainMenu();

	/**
	 * Limpia cualquier sesión residual o búsqueda en progreso al regresar al menú principal.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Network")
	void CleanupStaleSessions();

	/**
	 * Retorna la cantidad de sesiones encontradas tras la última búsqueda.
	 */
	UFUNCTION(BlueprintPure, Category = "PEV2|Network")
	int32 GetSearchResultsCount() const;

	/**
	 * Retorna el código de sala activo actual.
	 */
	UFUNCTION(BlueprintPure, Category = "PEV2|Network")
	FString GetCurrentRoomCode() const { return CurrentRoomCode; }

	// ==========================================
	// DELEGADOS DISPONIBLES PARA UMG
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Network|Events")
	FPEV2OnCreateSessionComplete OnCreateSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Network|Events")
	FPEV2OnFindSessionsComplete OnFindSessionsCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Network|Events")
	FPEV2OnJoinSessionComplete OnJoinSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Network|Events")
	FPEV2OnNetworkErrorOccurred OnNetworkErrorEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Network|Events")
	FPEV2OnRoomCodeUpdated OnRoomCodeUpdatedEvent;

	// ==========================================
	// CONFIGURACIÓN DE MAPAS
	// ==========================================

	/** Ruta del mapa del Lobby donde el Host viaja tras crear la sesión */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PEV2|Maps")
	FString LobbyMapPath;

	/** Ruta del mapa de la Arena de combate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PEV2|Maps")
	FString GameArenaMapPath;

	/** Ruta del mapa del Menú Principal para recuperación ante errores */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PEV2|Maps")
	FString MainMenuMapPath;

	/** Código de sala actual del host o de la partida conectada */
	UPROPERTY(BlueprintReadOnly, Category = "PEV2|Network")
	FString CurrentRoomCode;

protected:
	// ==========================================
	// CALLBACKS INTERNOS DE ONLINESUBSYSTEM
	// ==========================================

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	// Callbacks globales de errores de red del motor
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

private:
	/** Retorna la interfaz de sesiones resuelta según el contexto del mundo actual */
	IOnlineSessionPtr GetSessionInterface() const;

	/** Configuración de la sesión creada */
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

	/** Búsqueda de sesiones activa */
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	// Handles de delegados de OnlineSubsystem
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

	// Parámetros cacheados en caso de tener que destruir una sesión previa antes de hostear
	bool bPendingHost;
	int32 PendingMaxPlayers;
	bool bPendingIsLAN;
	FString PendingCustomCode;

	// Parámetros de búsqueda por código
	bool bPendingJoinByCode;
	FString PendingRoomCodeToJoin;
	bool bJoinByCodeLANAttempted;
	bool bJoinByCodeOnlineAttempted;

	// Parámetros de salida limpia y unión tras destrucción
	bool bPendingReturnToMainMenu;
	FTimerHandle ReturnToMainMenuTimerHandle;
	bool bPendingJoinSession;
	int32 PendingJoinSessionIndex;
};
