// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Variant_Shooter/UI/ShooterBulletCounterUI.h"
#include "UI/PEV2_GameHUDWidget.h"
#include "Characters/PEV2_Character.h"
#include "Gameplay/Components/PEV2_CombatComponent.h"
#include "Gameplay/Components/PEV2_HealthComponent.h"
#include "Gameplay/Components/PEV2_AbilityComponent.h"
#include "Gameplay/Items/PEV2_AbilityData.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "UI/PEV2_HUD.h"
#include "Core/PEV2_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Input/SVirtualJoystick.h"

APEV2_PlayerController::APEV2_PlayerController()
{
	// Cargar Input Mapping Contexts por defecto si no han sido asignados en Blueprint
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMCObj(TEXT("/Game/Input/IMC_Default"));
	if (DefaultIMCObj.Succeeded())
	{
		DefaultMappingContexts.Add(DefaultIMCObj.Object);
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> WeaponsIMCObj(TEXT("/Game/Variant_Shooter/Input/IMC_Weapons"));
	if (WeaponsIMCObj.Succeeded())
	{
		DefaultMappingContexts.Add(WeaponsIMCObj.Object);
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookIMCObj(TEXT("/Game/Input/IMC_MouseLook"));
	if (MouseLookIMCObj.Succeeded())
	{
		MobileExcludedMappingContexts.Add(MouseLookIMCObj.Object);
	}

	HUDWidgetClass = UPEV2_GameHUDWidget::StaticClass();
}

void APEV2_PlayerController::AddDefaultMappingContext()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	// Si el Blueprint sobreescribió la lista con un array vacío, cargar fallbacks en runtime
	if (DefaultMappingContexts.Num() == 0)
	{
		if (UInputMappingContext* FallbackIMC = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default")))
		{
			DefaultMappingContexts.Add(FallbackIMC);
		}
	}

	if (UInputMappingContext* FallbackWeaponsIMC = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Variant_Shooter/Input/IMC_Weapons.IMC_Weapons")))
	{
		if (!DefaultMappingContexts.Contains(FallbackWeaponsIMC))
		{
			DefaultMappingContexts.Add(FallbackWeaponsIMC);
		}
	}

	if (MobileExcludedMappingContexts.Num() == 0)
	{
		if (UInputMappingContext* FallbackMouseLook = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook")))
		{
			MobileExcludedMappingContexts.Add(FallbackMouseLook);
		}
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
	{
		if (CurrentContext && !Subsystem->HasMappingContext(CurrentContext))
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}

	if (!SVirtualJoystick::ShouldDisplayTouchInterface())
	{
		for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
		{
			if (CurrentContext && !Subsystem->HasMappingContext(CurrentContext))
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void APEV2_PlayerController::CreateHUDWidget()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!HUDWidgetClass || HUDWidgetClass->GetName().Contains(TEXT("UI_ShooterBulletCounter")))
	{
		HUDWidgetClass = UPEV2_GameHUDWidget::StaticClass();
	}

	if (HUDWidgetClass && !ActiveHUDWidget)
	{
		ActiveHUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (ActiveHUDWidget)
		{
			ActiveHUDWidget->AddToPlayerScreen(0);
		}
	}
}

void APEV2_PlayerController::BindToPawn(APawn* InPawn)
{
	if (!IsLocalPlayerController() || !InPawn)
	{
		return;
	}

	CreateHUDWidget();
	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		PEV2HUD->HideDeathScreen();
	}

	if (APEV2_Character* PEV2Char = Cast<APEV2_Character>(InPawn))
	{
		if (UPEV2_HealthComponent* HealthComp = PEV2Char->FindComponentByClass<UPEV2_HealthComponent>())
		{
			HealthComp->OnHealthChangedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleHealthChanged);
			HealthComp->OnHealthChangedEvent.AddDynamic(this, &APEV2_PlayerController::HandleHealthChanged);
			HandleHealthChanged(HealthComp, HealthComp->CurrentHealth, 0.0f, nullptr);

			HealthComp->OnShieldChangedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleShieldChanged);
			HealthComp->OnShieldChangedEvent.AddDynamic(this, &APEV2_PlayerController::HandleShieldChanged);
			HandleShieldChanged(HealthComp, HealthComp->CurrentShield, 0.0f, HealthComp->MaxShield);
		}

		if (UPEV2_CombatComponent* CombatComp = PEV2Char->GetCombatComponent())
		{
			CombatComp->OnEquippedWeaponChangedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleEquippedWeaponChanged);
			CombatComp->OnEquippedWeaponChangedEvent.AddDynamic(this, &APEV2_PlayerController::HandleEquippedWeaponChanged);

			if (APEV2_WeaponBase* CurrentWeapon = CombatComp->GetEquippedWeapon())
			{
				HandleEquippedWeaponChanged(CurrentWeapon);
			}
		}

		PEV2Char->OnWeaponTierChangedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleWeaponTierChanged);
		PEV2Char->OnWeaponTierChangedEvent.AddDynamic(this, &APEV2_PlayerController::HandleWeaponTierChanged);

		if (UPEV2_AbilityComponent* AbilityComp = PEV2Char->GetAbilityComponent())
		{
			AbilityComp->OnAbilityChangedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleAbilityChanged);
			AbilityComp->OnAbilityChangedEvent.AddDynamic(this, &APEV2_PlayerController::HandleAbilityChanged);

			AbilityComp->OnAbilityChargesChangedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleAbilityChargesChanged);
			AbilityComp->OnAbilityChargesChangedEvent.AddDynamic(this, &APEV2_PlayerController::HandleAbilityChargesChanged);

			AbilityComp->OnAbilityCooldownUpdatedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleAbilityCooldownUpdated);
			AbilityComp->OnAbilityCooldownUpdatedEvent.AddDynamic(this, &APEV2_PlayerController::HandleAbilityCooldownUpdated);

			if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
			{
				GameHUD->UpdateAbility(AbilityComp->GetCurrentAbility(), AbilityComp->GetRemainingCharges(), AbilityComp->GetRemainingCooldownTime());
			}
		}

		if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
		{
			GameHUD->InitializeRoster(PEV2Char->GetWeaponTierClasses(), PEV2Char->GetCurrentWeaponTier());
		}
	}
}

void APEV2_PlayerController::HandleAbilityChanged(UPEV2_AbilityData* NewAbility, int32 InitialCharges)
{
	if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
	{
		GameHUD->UpdateAbility(NewAbility, InitialCharges, 0.0f);
	}
}

void APEV2_PlayerController::HandleAbilityChargesChanged(int32 RemainingCharges, int32 MaxCharges)
{
	if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
	{
		APEV2_Character* PEV2Char = Cast<APEV2_Character>(GetPawn());
		UPEV2_AbilityData* CurrentAbility = PEV2Char && PEV2Char->GetAbilityComponent() ? PEV2Char->GetAbilityComponent()->GetCurrentAbility() : nullptr;
		const float RemainingCooldown = PEV2Char && PEV2Char->GetAbilityComponent() ? PEV2Char->GetAbilityComponent()->GetRemainingCooldownTime() : 0.0f;
		GameHUD->UpdateAbility(CurrentAbility, RemainingCharges, RemainingCooldown);
	}
}

void APEV2_PlayerController::HandleAbilityCooldownUpdated(float RemainingCooldown, float TotalDuration)
{
	if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
	{
		GameHUD->UpdateAbilityCooldown(RemainingCooldown, TotalDuration);
	}
}

void APEV2_PlayerController::HandleWeaponTierChanged(int32 CurrentTier, int32 MaxTiers)
{
	if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
	{
		GameHUD->SetActiveWeaponTier(CurrentTier);
	}
}

void APEV2_PlayerController::HandleEquippedWeaponChanged(APEV2_WeaponBase* NewWeapon)
{
	if (!NewWeapon)
	{
		return;
	}

	NewWeapon->OnAmmoChangedEvent.RemoveDynamic(this, &APEV2_PlayerController::HandleAmmoChanged);
	NewWeapon->OnAmmoChangedEvent.AddDynamic(this, &APEV2_PlayerController::HandleAmmoChanged);

	if (APEV2_Character* PEV2Char = Cast<APEV2_Character>(GetPawn()))
	{
		if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
		{
			GameHUD->SetActiveWeaponTier(PEV2Char->GetCurrentWeaponTier());
		}
	}

	HandleAmmoChanged(NewWeapon->GetCurrentAmmo(), NewWeapon->GetMaxMagazineAmmo());
}

void APEV2_PlayerController::HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
	{
		APEV2_Character* PEV2Char = Cast<APEV2_Character>(GetPawn());
		APEV2_WeaponBase* CurrentWeapon = PEV2Char && PEV2Char->GetCombatComponent() ? PEV2Char->GetCombatComponent()->GetEquippedWeapon() : nullptr;
		const bool bIsMelee = CurrentWeapon && CurrentWeapon->IsMeleeWeapon();
		const FText WeaponName = CurrentWeapon ? CurrentWeapon->GetWeaponName() : FText::GetEmpty();
		GameHUD->UpdateAmmo(CurrentAmmo, MaxAmmo, bIsMelee, WeaponName);
	}

	if (UShooterBulletCounterUI* BulletUI = Cast<UShooterBulletCounterUI>(ActiveHUDWidget))
	{
		BulletUI->BP_UpdateBulletCounter(MaxAmmo, CurrentAmmo);
	}
}

void APEV2_PlayerController::HandleHealthChanged(UPEV2_HealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType)
{
	if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
	{
		const float MaxHealth = HealthComp ? HealthComp->MaxHealth : 100.0f;
		const float LifePercent = HealthComp ? HealthComp->GetHealthPercent() : 1.0f;
		GameHUD->UpdateHealth(Health, MaxHealth, LifePercent);
	}

	if (UShooterBulletCounterUI* BulletUI = Cast<UShooterBulletCounterUI>(ActiveHUDWidget))
	{
		const float LifePercent = HealthComp ? HealthComp->GetHealthPercent() : 1.0f;
		BulletUI->BP_Damaged(LifePercent);
	}
}

void APEV2_PlayerController::HandleShieldChanged(UPEV2_HealthComponent* HealthComp, float Shield, float ShieldDelta, float MaxShield)
{
	if (UPEV2_GameHUDWidget* GameHUD = Cast<UPEV2_GameHUDWidget>(ActiveHUDWidget))
	{
		const float SafeMaxShield = (MaxShield > 0.0f) ? MaxShield : (HealthComp ? HealthComp->MaxShield : 50.0f);
		const float ShieldPercent = (SafeMaxShield > 0.0f) ? (Shield / SafeMaxShield) : 0.0f;
		GameHUD->UpdateShield(Shield, SafeMaxShield, ShieldPercent);
	}
}

void APEV2_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	AddDefaultMappingContext();

	if (InputComponent)
	{
		FInputKeyBinding& EscBinding = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &APEV2_PlayerController::TogglePauseMenu);
		EscBinding.bExecuteWhenPaused = true;

		FInputKeyBinding& PBinding = InputComponent->BindKey(EKeys::P, IE_Pressed, this, &APEV2_PlayerController::TogglePauseMenu);
		PBinding.bExecuteWhenPaused = true;
	}
}

void APEV2_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	AddDefaultMappingContext();
	CreateHUDWidget();
	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (IsLocalPlayerController())
	{
		if (!GetHUD() || !GetHUD()->IsA<APEV2_HUD>())
		{
			ClientSetHUD(APEV2_HUD::StaticClass());
		}

		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputModeData);
		bShowMouseCursor = false;

		if (GetPawn())
		{
			BindToPawn(GetPawn());
		}
	}
}

void APEV2_PlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	AddDefaultMappingContext();

	if (IsLocalPlayerController())
	{
		if (!GetHUD() || !GetHUD()->IsA<APEV2_HUD>())
		{
			ClientSetHUD(APEV2_HUD::StaticClass());
		}

		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputModeData);
		bShowMouseCursor = false;
	}

	BindToPawn(aPawn);
}

void APEV2_PlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	AddDefaultMappingContext();

	if (IsLocalPlayerController())
	{
		if (!GetHUD() || !GetHUD()->IsA<APEV2_HUD>())
		{
			ClientSetHUD(APEV2_HUD::StaticClass());
		}
	}

	BindToPawn(P);
}

void APEV2_PlayerController::Client_ReceiveHitFeedback_Implementation(float DamageAmount, FVector ImpactDirection)
{
	// 1. Activar viñeta roja e indicador direccional en el HUD
	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		PEV2HUD->TriggerDamageFeedback(DamageAmount, ImpactDirection);
	}

	// 2. Notificar al widget de vida de Epic (flasheo de daño y actualización de barra)
	if (UShooterBulletCounterUI* BulletUI = Cast<UShooterBulletCounterUI>(ActiveHUDWidget))
	{
		if (APawn* MyPawn = GetPawn())
		{
			if (UPEV2_HealthComponent* HC = MyPawn->FindComponentByClass<UPEV2_HealthComponent>())
			{
				BulletUI->BP_Damaged(HC->GetHealthPercent());
			}
		}
	}

	// 3. Evento Blueprint para extensiones visuales personalizadas
	OnReceiveHitFeedback(DamageAmount, ImpactDirection);
}

void APEV2_PlayerController::Client_NotifyHitConfirmed_Implementation(bool bIsHeadshot, bool bWasKill)
{
	// 1. Activar Hitmarker ('X') en el crosshair del atacante
	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		PEV2HUD->TriggerHitmarker(bIsHeadshot, bWasKill);
	}

	// 2. Evento Blueprint
	OnHitConfirmed(bIsHeadshot, bWasKill);
}

void APEV2_PlayerController::Client_NotifyKillScored_Implementation(int32 CurrentKills, int32 RequiredKills)
{
	// Hitmarker rojo de baja confirmada
	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		PEV2HUD->TriggerHitmarker(false, true);
	}

	OnKillScored(CurrentKills, RequiredKills);
}

void APEV2_PlayerController::Client_NotifyPlayerDied_Implementation(float RespawnDuration)
{
	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		PEV2HUD->ShowDeathScreen(RespawnDuration);
	}

	// Mantener el HUD visible para que el jugador conserve la noción de sus stats (0 HP, balas, arma actual)
	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void APEV2_PlayerController::Client_ShowMatchEndScreen_Implementation(bool bIsWinner)
{
	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		PEV2HUD->ShowMatchEndScreen(bIsWinner);
	}

	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	// Habilitar cursor para poder seleccionar "Volver al Lobby" o "Salir del Juego"
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
	bShowMouseCursor = true;

	OnShowMatchEndScreen(bIsWinner);
}

void APEV2_PlayerController::ReturnToLobby()
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/PEV2/Maps/L_Lobby")));
}

void APEV2_PlayerController::ReturnToMainMenu()
{
	if (UPEV2_GameInstance* GI = Cast<UPEV2_GameInstance>(GetGameInstance()))
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/PEV2/Maps/L_MainMenu")));
	}
}

void APEV2_PlayerController::TogglePauseMenu()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		// Si la partida finalizó, no alternar pausa
		if (PEV2HUD->IsMatchEnded())
		{
			return;
		}

		PEV2HUD->TogglePauseMenu();

		if (PEV2HUD->IsPauseMenuActive())
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputModeData.SetHideCursorDuringCapture(false);
			SetInputMode(InputModeData);
			bShowMouseCursor = true;
		}
		else
		{
			FInputModeGameOnly InputModeData;
			SetInputMode(InputModeData);
			bShowMouseCursor = false;
		}
	}
}

bool APEV2_PlayerController::IsPauseMenuActive() const
{
	if (APEV2_HUD* PEV2HUD = Cast<APEV2_HUD>(GetHUD()))
	{
		return PEV2HUD->IsPauseMenuActive();
	}
	return false;
}

void APEV2_PlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
