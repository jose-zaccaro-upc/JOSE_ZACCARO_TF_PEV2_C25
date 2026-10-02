// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_LobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "UI/PEV2_LobbyWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

APEV2_LobbyPlayerController::APEV2_LobbyPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// Pre-cargar WBP_LobbyMenu en tiempo de construcción para asegurar disponibilidad instantánea
	static ConstructorHelpers::FClassFinder<UUserWidget> LobbyWidgetObj(TEXT("/Game/PEV2/UI/WBP_LobbyMenu"));
	if (LobbyWidgetObj.Succeeded())
	{
		LobbyWidgetClass = LobbyWidgetObj.Class;
	}
}

void APEV2_LobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[PEV2_LobbyPlayerController] BeginPlay en %s (IsLocalController: %s)"),
		*GetName(), IsLocalController() ? TEXT("True") : TEXT("False"));

	if (IsLocalController())
	{
		SetupLobbyUI();
	}
}

void APEV2_LobbyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	UE_LOG(LogTemp, Log, TEXT("[PEV2_LobbyPlayerController] ReceivedPlayer en %s (IsLocalController: %s)"),
		*GetName(), IsLocalController() ? TEXT("True") : TEXT("False"));

	// Crucial para clientes de red: ReceivedPlayer se dispara cuando el viewport local se vincula al controller
	if (IsLocalController())
	{
		SetupLobbyUI();
	}
}

void APEV2_LobbyPlayerController::SetupLobbyUI()
{
	if (!IsLocalController())
	{
		return;
	}

	if (ActiveLobbyWidget != nullptr)
	{
		return;
	}

	bShowMouseCursor = true;
	FInputModeUIOnly InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);

	// Enfocar cámara si existe en el nivel de lobby
	if (UWorld* World = GetWorld())
	{
		AActor* CamActor = UGameplayStatics::GetActorOfClass(World, ACameraActor::StaticClass());
		if (CamActor)
		{
			SetViewTarget(CamActor);
		}
	}

	// Carga de respaldo si no se obtuvo en el constructor
	if (!LobbyWidgetClass)
	{
		LobbyWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/PEV2/UI/WBP_LobbyMenu.WBP_LobbyMenu_C"));
		if (!LobbyWidgetClass)
		{
			LobbyWidgetClass = UPEV2_LobbyWidget::StaticClass();
		}
	}

	if (LobbyWidgetClass)
	{
		ActiveLobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (ActiveLobbyWidget)
		{
			ActiveLobbyWidget->AddToViewport(100);
			UE_LOG(LogTemp, Log, TEXT("[PEV2_LobbyPlayerController] ActiveLobbyWidget creado y añadido al Viewport con éxito."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[PEV2_LobbyPlayerController] Error al crear ActiveLobbyWidget"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PEV2_LobbyPlayerController] No se pudo encontrar LobbyWidgetClass"));
	}
}
