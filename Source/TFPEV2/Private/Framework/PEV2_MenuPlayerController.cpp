// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_MenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

APEV2_MenuPlayerController::APEV2_MenuPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// Hard reference en el constructor para que el Cooker de Unreal lo incluya en builds empaquetadas
	static ConstructorHelpers::FClassFinder<UUserWidget> MenuWidgetFinder(TEXT("/Game/PEV2/UI/WBP_MainMenu"));
	if (MenuWidgetFinder.Succeeded())
	{
		MainMenuWidgetClass = MenuWidgetFinder.Class;
	}
}

void APEV2_MenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		bShowMouseCursor = true;
		FInputModeUIOnly InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputModeData);

		// Si no fue asignado en el Inspector o Blueprint, cargarlo desde Content/PEV2/UI/WBP_MainMenu
		if (!MainMenuWidgetClass)
		{
			MainMenuWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/PEV2/UI/WBP_MainMenu.WBP_MainMenu_C"));
		}

		if (MainMenuWidgetClass)
		{
			ActiveMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
			if (ActiveMenuWidget)
			{
				ActiveMenuWidget->AddToViewport(100);
			}
		}
	}
}
