// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "UI/PEV2_HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Framework/PEV2_PlayerController.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

APEV2_HUD::APEV2_HUD()
{
	bShowCrosshair = true;
	CrosshairSpread = 7.0f;
	CrosshairLength = 9.0f;
	CrosshairThickness = 2.0f;
	CrosshairColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.85f);
	bDrawCenterDot = true;

	HitmarkerDuration = 0.22f;
	HitmarkerLength = 7.0f;
	HitmarkerGap = 6.0f;
	HitmarkerThickness = 2.5f;
	NormalHitColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	HeadshotHitColor = FLinearColor(1.0f, 0.15f, 0.15f, 1.0f);
	KillHitColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	DamageFlashDecaySpeed = 3.5f;
	DamageVignetteColor = FLinearColor(0.85f, 0.0f, 0.0f, 1.0f);

	HitmarkerTimeRemaining = 0.0f;
	DamageFlashAlpha = 0.0f;
	DirectionalIndicatorTimer = 0.0f;

	bIsDeathScreenActive = false;
	DeathRemainingTime = 0.0f;
	DeathTitleText = FText::FromString(TEXT("HAS MUERTO"));
	DeathTextColor = FLinearColor(0.95f, 0.12f, 0.12f, 1.0f);

	bIsMatchEndScreenActive = false;
	bIsWinnerLocal = false;

	static ConstructorHelpers::FObjectFinder<UTexture2D> VictoryTex(TEXT("/Game/PEV2/Assets/Wallpapers/Victory.Victory"));
	if (VictoryTex.Succeeded())
	{
		VictoryWallpaper = VictoryTex.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> DefeatTex(TEXT("/Game/PEV2/Assets/Wallpapers/Defeat.Defeat"));
	if (DefeatTex.Succeeded())
	{
		DefeatWallpaper = DefeatTex.Object;
	}
}

void APEV2_HUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// 1. Pantalla de fin de partida (prioridad máxima)
	if (bIsMatchEndScreenActive)
	{
		DrawMatchEndScreen(DeltaSeconds);
		return;
	}

	// 2. Menú de pausa
	if (bIsPauseMenuActive)
	{
		DrawPauseMenu(DeltaSeconds);
		return;
	}

	// 3. Pantalla de muerte ("HAS MUERTO")
	if (bIsDeathScreenActive)
	{
		DrawDeathScreen(DeltaSeconds);
		return;
	}

	// 3. Viñeta roja de daño recibido
	DrawDamageVignette(DeltaSeconds);

	// 4. Indicador direccional de dónde provino el impacto
	DrawDirectionalDamageIndicator(Center, DeltaSeconds);

	// 5. Retícula (Crosshair) centrada
	if (bShowCrosshair)
	{
		DrawCrosshairLines(Center);
	}

	// 6. Hitmarker ('X') cuando confirmamos impacto en enemigo
	DrawHitmarker(Center, DeltaSeconds);
}

void APEV2_HUD::ShowDeathScreen(float InRespawnDuration)
{
	bIsDeathScreenActive = true;
	DeathRemainingTime = InRespawnDuration;
}

void APEV2_HUD::HideDeathScreen()
{
	bIsDeathScreenActive = false;
	DeathRemainingTime = 0.0f;
}

void APEV2_HUD::ShowMatchEndScreen(bool bInIsWinner)
{
	bIsMatchEndScreenActive = true;
	bIsWinnerLocal = bInIsWinner;
	bIsDeathScreenActive = false;
	bIsPauseMenuActive = false;
}

void APEV2_HUD::TogglePauseMenu()
{
	bIsPauseMenuActive = !bIsPauseMenuActive;
}

void APEV2_HUD::DrawDeathScreen(float DeltaSeconds)
{
	if (DeathRemainingTime > 0.0f)
	{
		DeathRemainingTime -= DeltaSeconds;
		if (DeathRemainingTime < 0.0f)
		{
			DeathRemainingTime = 0.0f;
		}
	}

	const float ScreenW = Canvas->ClipX;
	const float ScreenH = Canvas->ClipY;

	// Viñeta y oscurecimiento en pantalla
	DrawRect(FLinearColor(0.08f, 0.0f, 0.0f, 0.65f), 0.0f, 0.0f, ScreenW, ScreenH);
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f), 0.0f, 0.0f, ScreenW, 60.0f);
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f), 0.0f, ScreenH - 60.0f, ScreenW, 60.0f);

	// Título "HAS MUERTO"
	const FString TitleStr = DeathTitleText.ToString();
	float TitleW = 0.0f, TitleH = 0.0f;
	GetTextSize(TitleStr, TitleW, TitleH, GEngine->GetLargeFont(), 2.2f);

	const float TitleX = (ScreenW - TitleW) * 0.5f;
	const float TitleY = (ScreenH * 0.42f) - (TitleH * 0.5f);

	// Sombra del texto
	DrawText(TitleStr, FLinearColor::Black, TitleX + 3.0f, TitleY + 3.0f, GEngine->GetLargeFont(), 2.2f);
	// Texto principal
	DrawText(TitleStr, DeathTextColor, TitleX, TitleY, GEngine->GetLargeFont(), 2.2f);

	// Línea decorativa
	const float LineW = TitleW * 1.3f;
	DrawRect(FLinearColor(0.9f, 0.1f, 0.1f, 0.9f), (ScreenW - LineW) * 0.5f, TitleY + TitleH + 12.0f, LineW, 3.0f);

	// Cuenta regresiva de reaparición
	const FString SubtitleStr = FString::Printf(TEXT("Reapareciendo en %.1f s..."), DeathRemainingTime);
	float SubW = 0.0f, SubH = 0.0f;
	GetTextSize(SubtitleStr, SubW, SubH, GEngine->GetMediumFont(), 1.3f);

	const float SubX = (ScreenW - SubW) * 0.5f;
	const float SubY = TitleY + TitleH + 28.0f;

	DrawText(SubtitleStr, FLinearColor(0.0f, 0.0f, 0.0f, 0.9f), SubX + 2.0f, SubY + 2.0f, GEngine->GetMediumFont(), 1.3f);
	DrawText(SubtitleStr, FLinearColor(0.95f, 0.95f, 0.95f, 0.95f), SubX, SubY, GEngine->GetMediumFont(), 1.3f);
}

void APEV2_HUD::DrawMatchEndScreen(float DeltaSeconds)
{
	const float ScreenW = Canvas->ClipX;
	const float ScreenH = Canvas->ClipY;

	// Dibujar imagen de fondo correspondiente (Victory o Defeat)
	UTexture2D* TargetTex = bIsWinnerLocal ? VictoryWallpaper : DefeatWallpaper;
	if (TargetTex)
	{
		DrawTexture(TargetTex, 0.0f, 0.0f, ScreenW, ScreenH, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor::White);
	}
	else
	{
		FLinearColor BgColor = bIsWinnerLocal ? FLinearColor(0.05f, 0.25f, 0.08f, 1.0f) : FLinearColor(0.25f, 0.05f, 0.05f, 1.0f);
		DrawRect(BgColor, 0.0f, 0.0f, ScreenW, ScreenH);
	}

	// Panel oscurecido para botones
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f), 0.0f, ScreenH - 140.0f, ScreenW, 140.0f);

	// Botones interactivos en Canvas: "Volver al Lobby" y "Salir del Juego"
	LobbyButtonSize = FVector2D(240.0f, 55.0f);
	QuitButtonSize = FVector2D(200.0f, 55.0f);

	LobbyButtonPos = FVector2D((ScreenW * 0.5f) - LobbyButtonSize.X - 25.0f, ScreenH - 100.0f);
	QuitButtonPos = FVector2D((ScreenW * 0.5f) + 25.0f, ScreenH - 100.0f);

	// Detectar posición del mouse
	float MouseX = 0.0f, MouseY = 0.0f;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->GetMousePosition(MouseX, MouseY);
	}

	auto IsHovered = [MouseX, MouseY](const FVector2D& Pos, const FVector2D& Size)
	{
		return (MouseX >= Pos.X && MouseX <= Pos.X + Size.X && MouseY >= Pos.Y && MouseY <= Pos.Y + Size.Y);
	};

	const bool bLobbyHover = IsHovered(LobbyButtonPos, LobbyButtonSize);
	const bool bQuitHover = IsHovered(QuitButtonPos, QuitButtonSize);

	// Fondo y texto del botón "Volver al Lobby"
	FLinearColor LobbyBtnCol = bLobbyHover ? FLinearColor(0.2f, 0.6f, 0.95f, 0.95f) : FLinearColor(0.1f, 0.3f, 0.6f, 0.85f);
	DrawRect(LobbyBtnCol, LobbyButtonPos.X, LobbyButtonPos.Y, LobbyButtonSize.X, LobbyButtonSize.Y);

	const FString LobbyStr = TEXT("VOLVER AL LOBBY");
	float LW = 0.0f, LH = 0.0f;
	GetTextSize(LobbyStr, LW, LH, GEngine->GetMediumFont(), 1.1f);
	DrawText(LobbyStr, FLinearColor::White, LobbyButtonPos.X + (LobbyButtonSize.X - LW) * 0.5f, LobbyButtonPos.Y + (LobbyButtonSize.Y - LH) * 0.5f, GEngine->GetMediumFont(), 1.1f);

	// Fondo y texto del botón "Salir del Juego"
	FLinearColor QuitBtnCol = bQuitHover ? FLinearColor(0.85f, 0.2f, 0.2f, 0.95f) : FLinearColor(0.45f, 0.1f, 0.1f, 0.85f);
	DrawRect(QuitBtnCol, QuitButtonPos.X, QuitButtonPos.Y, QuitButtonSize.X, QuitButtonSize.Y);

	const FString QuitStr = TEXT("SALIR DEL JUEGO");
	float QW = 0.0f, QH = 0.0f;
	GetTextSize(QuitStr, QW, QH, GEngine->GetMediumFont(), 1.1f);
	DrawText(QuitStr, FLinearColor::White, QuitButtonPos.X + (QuitButtonSize.X - QW) * 0.5f, QuitButtonPos.Y + (QuitButtonSize.Y - QH) * 0.5f, GEngine->GetMediumFont(), 1.1f);

	// Si el jugador hace click sobre los botones
	if (APEV2_PlayerController* PEV2_PC = Cast<APEV2_PlayerController>(GetOwningPlayerController()))
	{
		if (PEV2_PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			if (bLobbyHover)
			{
				PEV2_PC->ReturnToLobby();
			}
			else if (bQuitHover)
			{
				PEV2_PC->QuitGame();
			}
		}
	}
}

void APEV2_HUD::TriggerHitmarker(bool bIsHeadshot, bool bWasKill)
{
	HitmarkerTimeRemaining = HitmarkerDuration;
	bLastHitWasHeadshot = bIsHeadshot;
	bLastHitWasKill = bWasKill;
}

void APEV2_HUD::TriggerDamageFeedback(float DamageAmount, FVector ImpactDirection)
{
	const float AddedAlpha = FMath::Clamp(DamageAmount / 40.0f, 0.35f, 0.75f);
	DamageFlashAlpha = FMath::Clamp(DamageFlashAlpha + AddedAlpha, 0.35f, 0.85f);

	if (!ImpactDirection.IsNearlyZero())
	{
		LastDamageDirection = ImpactDirection;
		DirectionalIndicatorTimer = 0.6f;
	}
}

void APEV2_HUD::DrawCrosshairLines(const FVector2D& Center)
{
	// Línea superior
	DrawLine(Center.X, Center.Y - CrosshairSpread, Center.X, Center.Y - CrosshairSpread - CrosshairLength, CrosshairColor, CrosshairThickness);
	// Línea inferior
	DrawLine(Center.X, Center.Y + CrosshairSpread, Center.X, Center.Y + CrosshairSpread + CrosshairLength, CrosshairColor, CrosshairThickness);
	// Línea izquierda
	DrawLine(Center.X - CrosshairSpread, Center.Y, Center.X - CrosshairSpread - CrosshairLength, Center.Y, CrosshairColor, CrosshairThickness);
	// Línea derecha
	DrawLine(Center.X + CrosshairSpread, Center.Y, Center.X + CrosshairSpread + CrosshairLength, Center.Y, CrosshairColor, CrosshairThickness);

	if (bDrawCenterDot)
	{
		DrawRect(CrosshairColor, Center.X - 1.0f, Center.Y - 1.0f, 2.0f, 2.0f);
	}
}

void APEV2_HUD::DrawHitmarker(const FVector2D& Center, float DeltaSeconds)
{
	if (HitmarkerTimeRemaining <= 0.0f)
	{
		return;
	}

	HitmarkerTimeRemaining -= DeltaSeconds;
	const float Alpha = FMath::Clamp(HitmarkerTimeRemaining / HitmarkerDuration, 0.0f, 1.0f);

	FLinearColor CurrentColor = NormalHitColor;
	if (bLastHitWasKill)
	{
		CurrentColor = KillHitColor;
	}
	else if (bLastHitWasHeadshot)
	{
		CurrentColor = HeadshotHitColor;
	}
	CurrentColor.A = Alpha;

	// Aspas diagonales ('X')
	DrawLine(Center.X - HitmarkerGap, Center.Y - HitmarkerGap, Center.X - HitmarkerGap - HitmarkerLength, Center.Y - HitmarkerGap - HitmarkerLength, CurrentColor, HitmarkerThickness);
	DrawLine(Center.X + HitmarkerGap, Center.Y - HitmarkerGap, Center.X + HitmarkerGap + HitmarkerLength, Center.Y - HitmarkerGap - HitmarkerLength, CurrentColor, HitmarkerThickness);
	DrawLine(Center.X - HitmarkerGap, Center.Y + HitmarkerGap, Center.X - HitmarkerGap - HitmarkerLength, Center.Y + HitmarkerGap + HitmarkerLength, CurrentColor, HitmarkerThickness);
	DrawLine(Center.X + HitmarkerGap, Center.Y + HitmarkerGap, Center.X + HitmarkerGap + HitmarkerLength, Center.Y + HitmarkerGap + HitmarkerLength, CurrentColor, HitmarkerThickness);
}

void APEV2_HUD::DrawDamageVignette(float DeltaSeconds)
{
	if (DamageFlashAlpha <= 0.0f)
	{
		return;
	}

	DamageFlashAlpha = FMath::FInterpTo(DamageFlashAlpha, 0.0f, DeltaSeconds, DamageFlashDecaySpeed);
	if (DamageFlashAlpha < 0.01f)
	{
		DamageFlashAlpha = 0.0f;
		return;
	}

	FLinearColor VignetteColor = DamageVignetteColor;
	VignetteColor.A = DamageFlashAlpha;

	const float BorderSize = 35.0f;
	const float ScreenW = Canvas->ClipX;
	const float ScreenH = Canvas->ClipY;

	// Borde superior
	DrawRect(VignetteColor, 0.0f, 0.0f, ScreenW, BorderSize);
	// Borde inferior
	DrawRect(VignetteColor, 0.0f, ScreenH - BorderSize, ScreenW, BorderSize);
	// Borde izquierdo
	DrawRect(VignetteColor, 0.0f, BorderSize, BorderSize, ScreenH - (BorderSize * 2.0f));
	// Borde derecho
	DrawRect(VignetteColor, ScreenW - BorderSize, BorderSize, BorderSize, ScreenH - (BorderSize * 2.0f));
}

void APEV2_HUD::DrawDirectionalDamageIndicator(const FVector2D& Center, float DeltaSeconds)
{
	if (DirectionalIndicatorTimer <= 0.0f)
	{
		return;
	}

	DirectionalIndicatorTimer -= DeltaSeconds;
	const float Alpha = FMath::Clamp(DirectionalIndicatorTimer / 0.6f, 0.0f, 1.0f);

	if (APlayerController* PC = GetOwningPlayerController())
	{
		const FRotator CamRot = PC->GetControlRotation();
		const FVector CamFwd = CamRot.Vector();
		const FVector CamRight = FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y);

		const FVector FlatImpact = FVector(LastDamageDirection.X, LastDamageDirection.Y, 0.0f).GetSafeNormal();
		const FVector FlatFwd = FVector(CamFwd.X, CamFwd.Y, 0.0f).GetSafeNormal();
		const FVector FlatRight = FVector(CamRight.X, CamRight.Y, 0.0f).GetSafeNormal();

		const float DotFwd = FVector::DotProduct(FlatImpact, FlatFwd);
		const float DotRight = FVector::DotProduct(FlatImpact, FlatRight);

		const float Angle = FMath::Atan2(-DotRight, -DotFwd);
		const float IndicatorRadius = 120.0f;

		const FVector2D IndicatorPos(
			Center.X + (FMath::Sin(Angle) * IndicatorRadius),
			Center.Y - (FMath::Cos(Angle) * IndicatorRadius)
		);

		FLinearColor ArrowColor = DamageVignetteColor;
		ArrowColor.A = Alpha;

		DrawRect(ArrowColor, IndicatorPos.X - 8.0f, IndicatorPos.Y - 8.0f, 16.0f, 16.0f);
	}
}

void APEV2_HUD::DrawPauseMenu(float DeltaSeconds)
{
	const float ScreenW = Canvas->ClipX;
	const float ScreenH = Canvas->ClipY;

	// Fondo oscuro semi-transparente
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f), 0.0f, 0.0f, ScreenW, ScreenH);

	// Contenedor central (Card)
	const float CardW = 380.0f;
	const float CardH = 430.0f;
	const float CardX = (ScreenW - CardW) * 0.5f;
	const float CardY = (ScreenH - CardH) * 0.5f;

	// Fondo del Card
	DrawRect(FLinearColor(0.06f, 0.09f, 0.14f, 0.95f), CardX, CardY, CardW, CardH);

	// Barra superior de acento (Cyan)
	DrawRect(FLinearColor(0.0f, 0.7f, 1.0f, 1.0f), CardX, CardY, CardW, 4.0f);

	// Borde fino del Card
	DrawLine(CardX, CardY, CardX + CardW, CardY, FLinearColor(0.2f, 0.3f, 0.45f, 0.8f), 1.0f);
	DrawLine(CardX, CardY + CardH, CardX + CardW, CardY + CardH, FLinearColor(0.2f, 0.3f, 0.45f, 0.8f), 1.0f);
	DrawLine(CardX, CardY, CardX, CardY + CardH, FLinearColor(0.2f, 0.3f, 0.45f, 0.8f), 1.0f);
	DrawLine(CardX + CardW, CardY, CardX + CardW, CardY + CardH, FLinearColor(0.2f, 0.3f, 0.45f, 0.8f), 1.0f);

	// Título "PAUSA"
	const FString TitleStr = TEXT("PAUSA");
	float TitleW = 0.0f, TitleH = 0.0f;
	GetTextSize(TitleStr, TitleW, TitleH, GEngine->GetLargeFont(), 1.6f);
	DrawText(TitleStr, FLinearColor::White, CardX + (CardW - TitleW) * 0.5f, CardY + 25.0f, GEngine->GetLargeFont(), 1.6f);

	// Línea separadora bajo el título
	DrawRect(FLinearColor(0.25f, 0.35f, 0.5f, 0.5f), CardX + 30.0f, CardY + 25.0f + TitleH + 10.0f, CardW - 60.0f, 1.0f);

	// Dimensiones de botones
	const float BtnW = 300.0f;
	const float BtnH = 48.0f;
	const float BtnX = CardX + (CardW - BtnW) * 0.5f;
	float StartY = CardY + 85.0f;
	const float Spacing = 60.0f;

	PauseResumeButtonPos = FVector2D(BtnX, StartY);
	PauseResumeButtonSize = FVector2D(BtnW, BtnH);

	PauseMainMenuButtonPos = FVector2D(BtnX, StartY + Spacing);
	PauseMainMenuButtonSize = FVector2D(BtnW, BtnH);

	PauseLobbyButtonPos = FVector2D(BtnX, StartY + Spacing * 2.0f);
	PauseLobbyButtonSize = FVector2D(BtnW, BtnH);

	PauseQuitButtonPos = FVector2D(BtnX, StartY + Spacing * 3.0f);
	PauseQuitButtonSize = FVector2D(BtnW, BtnH);

	// Detección de hover del mouse
	float MouseX = 0.0f, MouseY = 0.0f;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->GetMousePosition(MouseX, MouseY);
	}

	auto IsHovered = [MouseX, MouseY](const FVector2D& Pos, const FVector2D& Size)
	{
		return (MouseX >= Pos.X && MouseX <= Pos.X + Size.X && MouseY >= Pos.Y && MouseY <= Pos.Y + Size.Y);
	};

	const bool bResumeHover = IsHovered(PauseResumeButtonPos, PauseResumeButtonSize);
	const bool bMenuHover = IsHovered(PauseMainMenuButtonPos, PauseMainMenuButtonSize);
	const bool bLobbyHover = IsHovered(PauseLobbyButtonPos, PauseLobbyButtonSize);
	const bool bQuitHover = IsHovered(PauseQuitButtonPos, PauseQuitButtonSize);

	auto DrawButton = [this](const FVector2D& Pos, const FVector2D& Size, const FString& Text, bool bHover, const FLinearColor& NormalCol, const FLinearColor& HoverCol)
	{
		FLinearColor BgCol = bHover ? HoverCol : NormalCol;
		DrawRect(BgCol, Pos.X, Pos.Y, Size.X, Size.Y);

		float TW = 0.0f, TH = 0.0f;
		GetTextSize(Text, TW, TH, GEngine->GetMediumFont(), 1.05f);
		DrawText(Text, FLinearColor::White, Pos.X + (Size.X - TW) * 0.5f, Pos.Y + (Size.Y - TH) * 0.5f, GEngine->GetMediumFont(), 1.05f);
	};

	// 1. REANUDAR
	DrawButton(PauseResumeButtonPos, PauseResumeButtonSize, TEXT("REANUDAR"), bResumeHover,
		FLinearColor(0.08f, 0.35f, 0.55f, 0.85f), FLinearColor(0.12f, 0.55f, 0.85f, 0.98f));

	// 2. VOLVER AL MENÚ PRINCIPAL
	DrawButton(PauseMainMenuButtonPos, PauseMainMenuButtonSize, TEXT("VOLVER AL MENÚ"), bMenuHover,
		FLinearColor(0.12f, 0.35f, 0.35f, 0.85f), FLinearColor(0.18f, 0.55f, 0.55f, 0.98f));

	// 3. VOLVER AL LOBBY
	DrawButton(PauseLobbyButtonPos, PauseLobbyButtonSize, TEXT("VOLVER AL LOBBY"), bLobbyHover,
		FLinearColor(0.12f, 0.22f, 0.45f, 0.85f), FLinearColor(0.18f, 0.35f, 0.70f, 0.98f));

	// 4. SALIR DEL JUEGO
	DrawButton(PauseQuitButtonPos, PauseQuitButtonSize, TEXT("SALIR DEL JUEGO"), bQuitHover,
		FLinearColor(0.45f, 0.12f, 0.12f, 0.85f), FLinearColor(0.75f, 0.18f, 0.18f, 0.98f));

	// Procesar clicks del mouse
	if (APEV2_PlayerController* PEV2_PC = Cast<APEV2_PlayerController>(GetOwningPlayerController()))
	{
		if (PEV2_PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			if (bResumeHover)
			{
				PEV2_PC->TogglePauseMenu();
			}
			else if (bMenuHover)
			{
				PEV2_PC->ReturnToMainMenu();
			}
			else if (bLobbyHover)
			{
				PEV2_PC->ReturnToLobby();
			}
			else if (bQuitHover)
			{
				PEV2_PC->QuitGame();
			}
		}
	}
}
