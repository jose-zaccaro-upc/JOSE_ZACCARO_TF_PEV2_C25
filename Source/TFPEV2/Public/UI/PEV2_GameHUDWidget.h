// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/UI/ShooterBulletCounterUI.h"
#include "Styling/SlateBrush.h"
#include "PEV2_GameHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UBorder;
class UImage;
class UPanelWidget;
class APEV2_WeaponBase;
class UPEV2_AbilityData;
class SPEV2_GameHUD;

USTRUCT(BlueprintType)
struct FPEV2_HUDWeaponItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|HUD")
	int32 TierIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|HUD")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|HUD")
	TObjectPtr<UTexture2D> WeaponIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|HUD")
	bool bIsActive = false;
};

/**
 * Main gameplay HUD widget for Gun Master Arena.
 * Features:
 * - Health bar labeled "VIDA" with dynamic status color and current/max life.
 * - Ammo bar labeled "BALAS" with current/max count (or infinite for melee).
 * - Weapon roster dock ("recuadros") showing weapon icons, names, and highlighted current weapon.
 * Works seamlessly both out-of-the-box via Slate and through Blueprint UMG bindings.
 */
UCLASS()
class TFPEV2_API UPEV2_GameHUDWidget : public UShooterBulletCounterUI
{
	GENERATED_BODY()

public:
	UPEV2_GameHUDWidget(const FObjectInitializer& ObjectInitializer);

	/** Initializes or rebuilds the weapon roster dock from character classes */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void InitializeRoster(const TArray<TSubclassOf<APEV2_WeaponBase>>& WeaponClasses, int32 ActiveTierIndex);

	/** Highlights the active weapon tier card and dims the others */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void SetActiveWeaponTier(int32 ActiveTierIndex);

	/** Updates the health bar, "VIDA" label, and numerical status */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void UpdateHealth(float InCurrentHealth, float InMaxHealth, float InLifePercent);

	/** Updates the shield bar, "ESCUDO" label, and numerical status */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void UpdateShield(float InCurrentShield, float InMaxShield, float InShieldPercent);

	/** Updates the ammo bar, "BALAS" label, and magazine numerical status */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void UpdateAmmo(int32 InCurrentAmmo, int32 InMaxAmmo, bool bInIsMelee, const FText& InWeaponName);

	/** Updates the tactical ability card ("Q"), name, icon, charges, and cooldown */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void UpdateAbility(UPEV2_AbilityData* InAbilityData, int32 InRemainingCharges, float InRemainingCooldown);

	/** Updates dynamic cooldown timer and progress for the tactical ability */
	UFUNCTION(BlueprintCallable, Category = "PEV2|HUD")
	void UpdateAbilityCooldown(float InRemainingCooldown, float InTotalDuration);

	/** Generates or retrieves a clean weapon silhouette icon texture */
	static UTexture2D* GetOrCreateIconForWeapon(TSubclassOf<APEV2_WeaponBase> WeaponClass, const FString& FallbackName);

	/** Generates or retrieves a clean ability icon texture (Grenade, Dash, etc.) */
	static UTexture2D* GetOrCreateIconForAbility(UPEV2_AbilityData* AbilityData, const FString& FallbackName);

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// ==========================================
	// OPTIONAL BLUEPRINT UMG BINDINGS
	// ==========================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_Health;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_HealthLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_HealthValue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_Shield;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ShieldLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ShieldValue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_Ammo;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_AmmoLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_AmmoValue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_EquippedWeaponName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_WeaponRoster;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_AbilityCard;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_AbilityIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_AbilityName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_AbilityCharges;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_AbilityCooldown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_AbilityCooldown;

private:
	TSharedPtr<SPEV2_GameHUD> SlateHUDWidget;

	float CachedHealth = 100.0f;
	float CachedMaxHealth = 100.0f;
	float CachedHealthPercent = 1.0f;

	float CachedShield = 0.0f;
	float CachedMaxShield = 50.0f;
	float CachedShieldPercent = 0.0f;

	int32 CachedAmmo = 30;
	int32 CachedMaxAmmo = 30;
	bool bCachedIsMelee = false;
	FText CachedWeaponName;

	int32 CachedActiveTier = 0;
	TArray<FPEV2_HUDWeaponItem> CachedRosterItems;

	TWeakObjectPtr<UPEV2_AbilityData> CachedAbilityData;
	int32 CachedAbilityCharges = 0;
	float CachedRemainingCooldown = 0.0f;
	float CachedTotalCooldown = 0.0f;

	static TMap<FString, TObjectPtr<UTexture2D>> StaticWeaponIcons;
	static TMap<FString, TObjectPtr<UTexture2D>> StaticAbilityIcons;
};
