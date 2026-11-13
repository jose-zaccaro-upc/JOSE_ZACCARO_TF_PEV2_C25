// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFPEV2Character.h"
#include "Interfaces/PEV2_CombatInterface.h"
#include "Gameplay/Components/PEV2_HealthComponent.h"
#include "Gameplay/Components/PEV2_CombatComponent.h"
#include "Gameplay/Components/PEV2_AbilityComponent.h"
#include "PEV2_Character.generated.h"

class APEV2_WeaponBase;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponTierChanged, int32, CurrentTier, int32, MaxTiers);

/**
 * Personaje principal de combate para Gun Master Arena / Escalada de Armas.
 * Integra salud, armas replicadas con progresión automática y habilidades dinámicas.
 */
UCLASS()
class TFPEV2_API APEV2_Character : public ATFPEV2Character, public IPEV2_CombatInterface
{
	GENERATED_BODY()

public:
	APEV2_Character();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	virtual FRotator GetBaseAimRotation() const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	// ==========================================
	// IMPLEMENTACIÓN IPEV2_CombatInterface
	// ==========================================

	virtual UPEV2_CombatComponent* GetCombatComponent() const override { return CombatComponent; }
	virtual UPEV2_AbilityComponent* GetAbilityComponent() const override { return AbilityComponent; }
	virtual FVector GetCombatAimLocation() const override;
	virtual FName GetWeaponAttachSocket(bool bFirstPerson) const override;

	/** Plays elimination ragdoll and FX on all clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEliminationFX();

	/** Resets ragdoll and collision state upon respawn */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ResetRespawnFX();

	// ==========================================
	// ESCALADA DE ARMAS (ARMS RACE)
	// ==========================================

	/** Avanza al siguiente nivel de arma y equipa el arma correspondiente (Autoridad) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PEV2|ArmsRace")
	void AdvanceWeaponTier();

	/** Spawnea y equipa el arma correspondiente al nivel actual (Autoridad) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PEV2|ArmsRace")
	void EquipCurrentWeaponTier();

	/** Restablece vida, colisión y estado para reaparecer en la arena */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PEV2|ArmsRace")
	void ResetForRespawn();

	/** Sincroniza el nivel de arma guardado y la apariencia desde el PlayerState */
	void RestoreFromPlayerState(class APEV2_PlayerState* InPS);

	UFUNCTION(BlueprintPure, Category = "PEV2|ArmsRace")
	int32 GetCurrentWeaponTier() const { return CurrentWeaponIndex; }

	UFUNCTION(BlueprintPure, Category = "PEV2|ArmsRace")
	int32 GetMaxWeaponTiers() const { return WeaponTierClasses.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "PEV2|ArmsRace|Events")
	FOnWeaponTierChanged OnWeaponTierChangedEvent;

protected:
	virtual void BeginPlay() override;

	// ==========================================
	// COMPONENTES DE JUEGO REPLICADOS
	// ==========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PEV2|Components")
	TObjectPtr<UPEV2_HealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PEV2|Components")
	TObjectPtr<UPEV2_CombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PEV2|Components")
	TObjectPtr<UPEV2_AbilityComponent> AbilityComponent;

	// ==========================================
	// ARMS RACE REPLICACIÓN
	// ==========================================

	/** Nivel de arma actual del jugador (RepNotify para actualizar HUD y modelo de arma en clientes) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponIndex, BlueprintReadOnly, Category = "PEV2|ArmsRace")
	int32 CurrentWeaponIndex;

	UFUNCTION()
	void OnRep_CurrentWeaponIndex();

	/** Índice de apariencia visual del personaje (malla Manny/Quinn y variación de skin) */
	UPROPERTY(ReplicatedUsing = OnRep_MeshAppearanceIndex, BlueprintReadOnly, Category = "PEV2|Appearance")
	int32 MeshAppearanceIndex;

	UFUNCTION()
	void OnRep_MeshAppearanceIndex();

	/** Aplica la apariencia según el índice MeshAppearanceIndex */
	void ApplyCharacterAppearance();

	/** Lista de clases de armas en orden de progresión para el modo Escalada de Armas */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|ArmsRace")
	TArray<TSubclassOf<APEV2_WeaponBase>> WeaponTierClasses;

	// ==========================================
	// SINCRONIZACIÓN DE APUNTADO VERTICAL (AIM PITCH)
	// ==========================================

	/** Ángulo de inclinación vertical de apuntado replicado a todos los clientes */
	UPROPERTY(ReplicatedUsing = OnRep_AimPitch, BlueprintReadOnly, Category = "PEV2|Aim")
	float ReplicatedAimPitch;

	UFUNCTION()
	void OnRep_AimPitch();

	UFUNCTION(Server, Unreliable)
	void Server_SetAimPitch(float NewPitch);

	// ==========================================
	// ACCIONES DE ENTRADA (ENHANCED INPUT)
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Input")
	TObjectPtr<UInputAction> UseAbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Input")
	TObjectPtr<UInputAction> InteractAction;

	// ==========================================
	// CÁMARA DE MUERTE EN TERCERA PERSONA
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|DeathCamera")
	float DeathCameraDistance = 280.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|DeathCamera")
	float DeathCameraHeightOffset = 130.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|DeathCamera")
	float DeathCameraInterpSpeed = 4.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|DeathCamera")
	FName RagdollFocusBone = FName(TEXT("pelvis"));

	// Handlers de Input
	void DoFireStart();
	void DoFireEnd();
	void DoReload();
	void DoUseAbility();
	void DoInteract();

	UFUNCTION()
	void OnDeath(AController* Killer, AActor* DamageCauser, const UDamageType* DamageType);

private:
	bool bIsDeadCameraActive;
	FVector DeathCameraCurrentLoc;
	FRotator DeathCameraCurrentRot;
	void UpdateAimPitchReplication();
};
