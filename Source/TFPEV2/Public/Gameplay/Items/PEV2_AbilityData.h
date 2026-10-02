// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PEV2_AbilityData.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UTexture2D;
class USoundBase;
class UNiagaraSystem;
class APEV2_ProjectileBase;

/**
 * Tipos de habilidades dinámicas disponibles para recoger del suelo.
 */
UENUM(BlueprintType)
enum class EPEV2_AbilityType : uint8
{
	None        UMETA(DisplayName = "Ninguna"),
	Dash        UMETA(DisplayName = "Dash (Desplazamiento Rápido)"),
	Grenade     UMETA(DisplayName = "Granada de Fragmentación"),
	Shield      UMETA(DisplayName = "Escudo Protector"),
	SpeedBoost  UMETA(DisplayName = "Aumento de Velocidad")
};

/**
 * Data Asset configurable en el editor de Unreal para definir cualquier habilidad dinámica y su apariencia en suelo.
 */
UCLASS(BlueprintType)
class TFPEV2_API UPEV2_AbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// CONFIGURACIÓN GENERAL
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	EPEV2_AbilityType AbilityType = EPEV2_AbilityType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FText AbilityName = FText::FromString(TEXT("Habilidad"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FText AbilityDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	TObjectPtr<UTexture2D> AbilityIcon;

	// ==========================================
	// BALANCE Y COOLDOWN
	// ==========================================

	/** Cantidad de cargas disponibles antes de agotar la habilidad o reiniciar */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxCharges = 1;

	/** Tiempo en segundos de recarga o cooldown entre activaciones */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance", meta = (ClampMin = "0.1", ClampMax = "60.0", Units = "s"))
	float CooldownDuration = 4.0f;

	// ==========================================
	// ESTÉTICA DEL PICKUP EN EL MUNDO
	// ==========================================

	/** Malla 3D del objeto flotante en el suelo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|Pickup")
	TObjectPtr<UStaticMesh> PickupMesh;

	/** Material opcional para sobrescribir la malla */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|Pickup")
	TObjectPtr<UMaterialInterface> PickupMaterialOverride;

	/** Color de la luz brillante y partículas del pickup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|Pickup")
	FLinearColor PickupGlowColor = FLinearColor(0.1f, 0.8f, 1.0f);

	// ==========================================
	// EFECTOS ESPECÍFICOS DE CADA HABILIDAD
	// ==========================================

	/** Fuerza del impulso para la habilidad Dash */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|Dash", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::Dash", Units = "cm/s"))
	float DashStrength = 2400.0f;

	/** Si es true, el dash se hace hacia la dirección que mira la cámara en vez del plano del suelo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|Dash", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::Dash"))
	bool bDashInAimDirection = false;

	/** Clase de proyectil para la Granada */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|Grenade", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::Grenade"))
	TSubclassOf<APEV2_ProjectileBase> GrenadeProjectileClass;

	/** Fuerza con la que se arroja la granada */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|Grenade", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::Grenade", Units = "cm/s"))
	float GrenadeThrowImpulse = 1800.0f;

	/** Cantidad de escudo temporal a sumar */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|Shield", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::Shield"))
	float ShieldBonusAmount = 50.0f;

	/** Duración del escudo temporal en segundos */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|Shield", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::Shield", Units = "s"))
	float ShieldDuration = 8.0f;

	/** Multiplicador de velocidad de movimiento (ej. 1.6 = +60%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|SpeedBoost", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::SpeedBoost"))
	float SpeedMultiplier = 1.5f;

	/** Duración del aumento de velocidad en segundos */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|SpeedBoost", meta = (EditCondition = "AbilityType == EPEV2_AbilityType::SpeedBoost", Units = "s"))
	float SpeedBoostDuration = 5.0f;

	// ==========================================
	// AUDIO Y PARTICULAS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AudioVisual")
	TObjectPtr<USoundBase> ActivationSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AudioVisual")
	TObjectPtr<UNiagaraSystem> ActivationVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AudioVisual")
	TObjectPtr<USoundBase> PickupSound;
};
