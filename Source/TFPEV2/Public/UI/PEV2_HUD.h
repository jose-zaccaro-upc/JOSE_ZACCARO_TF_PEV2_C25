// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PEV2_HUD.generated.h"

/**
 * HUD principal para Gun Master Arena.
 * Renderiza crosshair dinámico de alta fidelidad, hitmarkers (confirmación de impacto al disparar),
 * y viñeta/indicadores de daño recibido al ser impactado.
 */
UCLASS()
class TFPEV2_API APEV2_HUD : public AHUD
{
	GENERATED_BODY()

public:
	APEV2_HUD();

	virtual void DrawHUD() override;

	/** Activa el Hitmarker en el centro de la retícula al impactar a un enemigo */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void TriggerHitmarker(bool bIsHeadshot, bool bWasKill);

	/** Activa el efecto visual de daño recibido (viñeta roja e indicador de dirección) */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void TriggerDamageFeedback(float DamageAmount, FVector ImpactDirection);

	/** Activa la pantalla de muerte "HAS MUERTO" y cuenta regresiva */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void ShowDeathScreen(float InRespawnDuration);

	/** Desactiva la pantalla de muerte tras reaparecer */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void HideDeathScreen();

	/** Activa la pantalla de fin de partida (Victoria o Derrota) */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void ShowMatchEndScreen(bool bInIsWinner);

	/** Alterna la visibilidad del menú de pausa */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void TogglePauseMenu();

	/** Indica si el menú de pausa está activo */
	UFUNCTION(BlueprintPure, Category = "PEV2|HUD")
	bool IsPauseMenuActive() const { return bIsPauseMenuActive; }

	/** Indica si la partida ha finalizado */
	UFUNCTION(BlueprintPure, Category = "PEV2|HUD")
	bool IsMatchEnded() const { return bIsMatchEndScreenActive; }

protected:
	// ==========================================
	// CONFIGURACIÓN DE RETÍCULA (CROSSHAIR)
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Crosshair")
	bool bShowCrosshair = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Crosshair")
	float CrosshairSpread = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Crosshair")
	float CrosshairLength = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Crosshair")
	float CrosshairThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Crosshair")
	FLinearColor CrosshairColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Crosshair")
	bool bDrawCenterDot = true;

	// ==========================================
	// CONFIGURACIÓN DE HITMARKER
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Hitmarker")
	float HitmarkerDuration = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Hitmarker")
	float HitmarkerLength = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Hitmarker")
	float HitmarkerGap = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Hitmarker")
	float HitmarkerThickness = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Hitmarker")
	FLinearColor NormalHitColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Hitmarker")
	FLinearColor HeadshotHitColor = FLinearColor(1.0f, 0.15f, 0.15f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Hitmarker")
	FLinearColor KillHitColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	// ==========================================
	// CONFIGURACIÓN DE FEEDBACK DE DAÑO RECIBIDO
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|DamageFeedback")
	float DamageFlashDecaySpeed = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|DamageFeedback")
	FLinearColor DamageVignetteColor = FLinearColor(0.85f, 0.0f, 0.0f, 1.0f);

	// ==========================================
	// PANTALLA DE MUERTE ("HAS MUERTO")
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|DeathUI")
	FText DeathTitleText = FText::FromString(TEXT("HAS MUERTO"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|DeathUI")
	FLinearColor DeathTextColor = FLinearColor(0.95f, 0.12f, 0.12f, 1.0f);

	// ==========================================
	// PANTALLA DE FIN DE PARTIDA (VICTORIA / DERROTA)
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|MatchEnd")
	TObjectPtr<UTexture2D> VictoryWallpaper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|MatchEnd")
	TObjectPtr<UTexture2D> DefeatWallpaper;

private:
	void DrawCrosshairLines(const FVector2D& Center);
	void DrawHitmarker(const FVector2D& Center, float DeltaSeconds);
	void DrawDamageVignette(float DeltaSeconds);
	void DrawDirectionalDamageIndicator(const FVector2D& Center, float DeltaSeconds);
	void DrawDeathScreen(float DeltaSeconds);
	void DrawMatchEndScreen(float DeltaSeconds);
	void DrawPauseMenu(float DeltaSeconds);

	// Estado interno dinámico
	float HitmarkerTimeRemaining = 0.0f;
	bool bLastHitWasHeadshot = false;
	bool bLastHitWasKill = false;

	float DamageFlashAlpha = 0.0f;
	float DirectionalIndicatorTimer = 0.0f;
	FVector LastDamageDirection = FVector::ZeroVector;

	// Estado de muerte
	bool bIsDeathScreenActive = false;
	float DeathRemainingTime = 0.0f;

	// Estado de fin de partida
	bool bIsMatchEndScreenActive = false;
	bool bIsWinnerLocal = false;
	FVector2D LobbyButtonPos;
	FVector2D LobbyButtonSize;
	FVector2D QuitButtonPos;
	FVector2D QuitButtonSize;

	// Estado del menú de pausa
	bool bIsPauseMenuActive = false;
	FVector2D PauseResumeButtonPos;
	FVector2D PauseResumeButtonSize;
	FVector2D PauseMainMenuButtonPos;
	FVector2D PauseMainMenuButtonSize;
	FVector2D PauseLobbyButtonPos;
	FVector2D PauseLobbyButtonSize;
	FVector2D PauseQuitButtonPos;
	FVector2D PauseQuitButtonSize;
};
