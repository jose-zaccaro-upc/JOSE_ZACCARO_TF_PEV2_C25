// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFPEV2PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "Gameplay/Components/PEV2_HealthComponent.h"
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

	/** Widget class to use for the crosshair and HUD */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	/** Instantiated HUD widget (contains Crosshair, Ammo, Health) */
	UPROPERTY(BlueprintReadOnly, Category = "PEV2|UI")
	TObjectPtr<UUserWidget> ActiveHUDWidget;

	/** Creates the HUD widget (crosshair) if not already created */
	void CreateHUDWidget();

	/** Called on the owning client when taking damage for UI/Audio feedback */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "PEV2|Feedback")
	void Client_ReceiveHitFeedback(float DamageAmount, FVector ImpactDirection);
	
	/** Expose a Blueprint Implementable Event so UMG can react to hit feedback */
	UFUNCTION(BlueprintImplementableEvent, Category = "PEV2|Feedback")
	void OnReceiveHitFeedback(float DamageAmount, FVector ImpactDirection);

	/** Called on the attacker's client when a shot successfully deals damage */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "PEV2|Feedback")
	void Client_NotifyHitConfirmed(bool bIsHeadshot, bool bWasKill);

	/** Expose to UMG for crosshair hitmarker and audio */
	UFUNCTION(BlueprintImplementableEvent, Category = "PEV2|Feedback")
	void OnHitConfirmed(bool bIsHeadshot, bool bWasKill);

	/** Called on the attacker's client when scoring a kill */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "PEV2|Feedback")
	void Client_NotifyKillScored(int32 CurrentKills, int32 RequiredKills);

	/** Expose to UMG for kill banner and notification */
	UFUNCTION(BlueprintImplementableEvent, Category = "PEV2|Feedback")
	void OnKillScored(int32 CurrentKills, int32 RequiredKills);

	/** Notifica la muerte al jugador local con el tiempo de reaparición */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "PEV2|Death")
	void Client_NotifyPlayerDied(float RespawnDuration);

	/** Notifica el fin de partida con pantalla de Victoria o Derrota */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "PEV2|MatchEnd")
	void Client_ShowMatchEndScreen(bool bIsWinner);

	UFUNCTION(BlueprintCallable, Category = "PEV2|MatchEnd")
	void ReturnToLobby();

	UFUNCTION(BlueprintCallable, Category = "PEV2|MatchEnd")
	void QuitGame();

	/** Widget class opcional para la pantalla de fin de partida */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|UI")
	TSubclassOf<UUserWidget> MatchEndWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "PEV2|UI")
	TObjectPtr<UUserWidget> ActiveMatchEndWidget;

	UFUNCTION(BlueprintImplementableEvent, Category = "PEV2|MatchEnd")
	void OnShowMatchEndScreen(bool bIsWinner);

	UFUNCTION()
	void HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);

	UFUNCTION()
	void HandleHealthChanged(UPEV2_HealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType);

	UFUNCTION()
	void HandleShieldChanged(UPEV2_HealthComponent* HealthComp, float Shield, float ShieldDelta, float MaxShield);

	UFUNCTION()
	void HandleEquippedWeaponChanged(APEV2_WeaponBase* NewWeapon);

	UFUNCTION()
	void HandleWeaponTierChanged(int32 CurrentTier, int32 MaxTiers);

	UFUNCTION()
	void HandleAbilityChanged(class UPEV2_AbilityData* NewAbility, int32 InitialCharges);

	UFUNCTION()
	void HandleAbilityChargesChanged(int32 RemainingCharges, int32 MaxCharges);

	UFUNCTION()
	void HandleAbilityCooldownUpdated(float RemainingCooldown, float TotalDuration);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

	void BindToPawn(APawn* InPawn);
};
