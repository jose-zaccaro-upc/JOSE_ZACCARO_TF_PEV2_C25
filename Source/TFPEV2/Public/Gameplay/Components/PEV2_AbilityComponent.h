// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Items/PEV2_AbilityData.h"
#include "PEV2_AbilityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityChanged, UPEV2_AbilityData*, NewAbility, int32, InitialCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityChargesChanged, int32, RemainingCharges, int32, MaxCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCooldownUpdated, float, RemainingCooldown, float, TotalDuration);

/**
 * Componente que gestiona el slot único de habilidad dinámica y su replicación en red.
 */
UCLASS(ClassGroup = (PEV2), meta = (BlueprintSpawnableComponent))
class TFPEV2_API UPEV2_AbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPEV2_AbilityComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// FUNCIONES DE JUGABILIDAD
	// ==========================================

	/**
	 * Intenta activar la habilidad equipada actualmente. Llamado por el input local (tecla 'Q').
	 */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Ability")
	void UseAbility();

	/**
	 * Asigna una nueva habilidad al personaje (Solo ejecutable en Servidor con autoridad).
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PEV2|Ability")
	void EquipAbility(UPEV2_AbilityData* NewAbility);

	/**
	 * Verifica si el jugador puede recoger una nueva habilidad del suelo.
	 * (Falso si aún tiene una habilidad activa con cargas o cooldown).
	 */
	UFUNCTION(BlueprintPure, Category = "PEV2|Ability")
	bool CanPickupAbility() const;

	/**
	 * Verifica si la habilidad actual está lista para ejecutarse (cargas > 0 y cooldown listo).
	 */
	UFUNCTION(BlueprintPure, Category = "PEV2|Ability")
	bool CanUseAbility() const;

	/** Retorna los datos de la habilidad equipada actualmente */
	UFUNCTION(BlueprintPure, Category = "PEV2|Ability")
	UPEV2_AbilityData* GetCurrentAbility() const { return CurrentAbilityData; }

	/** Retorna la cantidad de cargas restantes */
	UFUNCTION(BlueprintPure, Category = "PEV2|Ability")
	int32 GetRemainingCharges() const { return RemainingCharges; }

	/** Retorna el tiempo restante de cooldown en segundos */
	UFUNCTION(BlueprintPure, Category = "PEV2|Ability")
	float GetRemainingCooldownTime() const;

	// ==========================================
	// EVENTOS PARA UI / HUD
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Ability|Events")
	FOnAbilityChanged OnAbilityChangedEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Ability|Events")
	FOnAbilityChargesChanged OnAbilityChargesChangedEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Ability|Events")
	FOnAbilityCooldownUpdated OnAbilityCooldownUpdatedEvent;

protected:
	virtual void BeginPlay() override;

	// ==========================================
	// REPLICACIÓN DE VARIABLES
	// ==========================================

	UPROPERTY(ReplicatedUsing = OnRep_CurrentAbilityData, BlueprintReadOnly, Category = "PEV2|Ability")
	TObjectPtr<UPEV2_AbilityData> CurrentAbilityData;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingCharges, BlueprintReadOnly, Category = "PEV2|Ability")
	int32 RemainingCharges;

	UPROPERTY(ReplicatedUsing = OnRep_CooldownEndTime, BlueprintReadOnly, Category = "PEV2|Ability")
	float CooldownEndTime;

	UFUNCTION()
	void OnRep_CurrentAbilityData();

	UFUNCTION()
	void OnRep_RemainingCharges();

	UFUNCTION()
	void OnRep_CooldownEndTime();

	// ==========================================
	// RPCs DE RED
	// ==========================================

	/** Servidor valida y ejecuta la habilidad */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UseAbility();

	/** Multicast para reproducir sonido y partículas a todos los clientes */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAbilityFX(UPEV2_AbilityData* AbilityData);

	/** Ejecución de la lógica de cada tipo de habilidad en el servidor */
	void ExecuteAbilityLogic(UPEV2_AbilityData* AbilityData);

private:
	// Manejo de temporizador para efectos continuos (ej. SpeedBoost)
	FTimerHandle SpeedBoostTimerHandle;
	float OriginalWalkSpeed;

	void ResetSpeedBoost();
};
