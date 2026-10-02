// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFPEV2PlayerController.h"
#include "PEV2_PlayerController.generated.h"

/**
 * Main PlayerController for the Battle Zone match.
 */
UCLASS()
class TFPEV2_API APEV2_PlayerController : public ATFPEV2PlayerController
{
	GENERATED_BODY()

public:
	APEV2_PlayerController();

	/** Ensures the Enhanced Input Mapping Contexts are registered on the local player */
	void AddDefaultMappingContext();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

	/** Called on the owning client when taking damage for UI/Audio feedback */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "PEV2|Feedback")
	void Client_ReceiveHitFeedback(float DamageAmount, FVector ImpactDirection);
	
	/** Expose a Blueprint Implementable Event so UMG can react to hit feedback */
	UFUNCTION(BlueprintImplementableEvent, Category = "PEV2|Feedback")
	void OnReceiveHitFeedback(float DamageAmount, FVector ImpactDirection);
};
