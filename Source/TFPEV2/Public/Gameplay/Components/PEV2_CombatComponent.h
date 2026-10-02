// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "PEV2_CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedWeaponChanged, APEV2_WeaponBase*, NewWeapon);

/**
 * Componente que gestiona el inventario de armas del personaje y coordina el disparo y recarga replicados.
 */
UCLASS(ClassGroup = (PEV2), meta = (BlueprintSpawnableComponent))
class TFPEV2_API UPEV2_CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPEV2_CombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==========================================
	// ACCIONES DE COMBATE
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "PEV2|Combat")
	void StartFiring();

	UFUNCTION(BlueprintCallable, Category = "PEV2|Combat")
	void StopFiring();

	UFUNCTION(BlueprintCallable, Category = "PEV2|Combat")
	void Reload();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PEV2|Combat")
	void EquipWeapon(APEV2_WeaponBase* NewWeapon);

	UFUNCTION(BlueprintPure, Category = "PEV2|Combat")
	APEV2_WeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Combat|Events")
	FOnEquippedWeaponChanged OnEquippedWeaponChangedEvent;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadOnly, Category = "PEV2|Combat")
	TObjectPtr<APEV2_WeaponBase> EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Combat")
	FName WeaponAttachSocket = FName(TEXT("HandGrip_R"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Combat")
	TSubclassOf<APEV2_WeaponBase> DefaultWeaponClass;

	virtual void BeginPlay() override;
};
