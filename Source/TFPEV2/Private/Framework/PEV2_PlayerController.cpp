// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SVirtualJoystick.h"

APEV2_PlayerController::APEV2_PlayerController()
{
	// Cargar Input Mapping Contexts por defecto si no han sido asignados en Blueprint
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMCObj(TEXT("/Game/Input/IMC_Default"));
	if (DefaultIMCObj.Succeeded())
	{
		DefaultMappingContexts.Add(DefaultIMCObj.Object);
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookIMCObj(TEXT("/Game/Input/IMC_MouseLook"));
	if (MouseLookIMCObj.Succeeded())
	{
		MobileExcludedMappingContexts.Add(MouseLookIMCObj.Object);
	}
}

void APEV2_PlayerController::AddDefaultMappingContext()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	// Si el Blueprint sobreescribió la lista con un array vacío, cargar fallback en runtime
	if (DefaultMappingContexts.Num() == 0)
	{
		if (UInputMappingContext* FallbackIMC = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default")))
		{
			DefaultMappingContexts.Add(FallbackIMC);
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

void APEV2_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	AddDefaultMappingContext();
}

void APEV2_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	AddDefaultMappingContext();

	if (IsLocalPlayerController())
	{
		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputModeData);
		bShowMouseCursor = false;
	}
}

void APEV2_PlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	AddDefaultMappingContext();

	if (IsLocalPlayerController())
	{
		FInputModeGameOnly InputModeData;
		InputModeData.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputModeData);
		bShowMouseCursor = false;
	}
}

void APEV2_PlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	AddDefaultMappingContext();
}

void APEV2_PlayerController::Client_ReceiveHitFeedback_Implementation(float DamageAmount, FVector ImpactDirection)
{
	// Trigger the Blueprint implementable event so we can spawn HUD effects (e.g., blood vignette, hit markers)
	OnReceiveHitFeedback(DamageAmount, ImpactDirection);
}
