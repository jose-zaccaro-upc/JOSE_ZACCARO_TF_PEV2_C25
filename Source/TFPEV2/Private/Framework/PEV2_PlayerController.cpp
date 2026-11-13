// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Variant_Shooter/UI/ShooterBulletCounterUI.h"
#include "Characters/PEV2_Character.h"
#include "Gameplay/Components/PEV2_CombatComponent.h"
#include "Gameplay/Components/PEV2_HealthComponent.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "UI/PEV2_HUD.h"
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

	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultHUDWidgetObj(TEXT("/Game/Variant_Shooter/UI/UI_ShooterBulletCounter.UI_ShooterBulletCounter_C"));
	if (DefaultHUDWidgetObj.Succeeded())
	{
		HUDWidgetClass = DefaultHUDWidgetObj.Class;
	}
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

	if (!HUDWidgetClass)
	{
		if (UClass* FallbackHUDClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Variant_Shooter/UI/UI_ShooterBulletCounter.UI_ShooterBulletCounter_C")))
		{
			HUDWidgetClass = FallbackHUDClass;
		}
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

	HandleAmmoChanged(NewWeapon->GetCurrentAmmo(), NewWeapon->GetMaxMagazineAmmo());
}

void APEV2_PlayerController::HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (UShooterBulletCounterUI* BulletUI = Cast<UShooterBulletCounterUI>(ActiveHUDWidget))
	{
		BulletUI->BP_UpdateBulletCounter(MaxAmmo, CurrentAmmo);
	}
}

void APEV2_PlayerController::HandleHealthChanged(UPEV2_HealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType)
{
	if (UShooterBulletCounterUI* BulletUI = Cast<UShooterBulletCounterUI>(ActiveHUDWidget))
	{
		const float LifePercent = HealthComp ? HealthComp->GetHealthPercent() : 1.0f;
		BulletUI->BP_Damaged(LifePercent);
	}
}

void APEV2_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	AddDefaultMappingContext();
}

void APEV2_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	AddDefaultMappingContext();
	CreateHUDWidget();

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

	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->SetVisibility(ESlateVisibility::Hidden);
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

void APEV2_PlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
