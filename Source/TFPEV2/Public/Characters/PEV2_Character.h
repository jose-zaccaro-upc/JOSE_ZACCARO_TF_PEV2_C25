// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TFPEV2Character.h"
#include "Interfaces/PEV2_CombatInterface.h"
#include "PEV2_Character.generated.h"

class UPEV2_HealthComponent;
class UPEV2_CombatComponent;
class UPEV2_AbilityComponent;
class UInputAction;

/**
 * Personaje principal de combate para Gun Master Arena.
 * Integra salud, armas replicadas y el sistema de habilidad dinámica con tecla única.
 */
UCLASS()
class TFPEV2_API APEV2_Character : public ATFPEV2Character, public IPEV2_CombatInterface
{
	GENERATED_BODY()

public:
	APEV2_Character();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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

	// Handlers de Input
	void DoFireStart();
	void DoFireEnd();
	void DoReload();
	void DoUseAbility();
	void DoInteract();

	UFUNCTION()
	void OnDeath(AController* Killer, AActor* DamageCauser, const UDamageType* DamageType);
};
