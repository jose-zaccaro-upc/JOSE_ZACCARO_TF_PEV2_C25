// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PEV2_WeaponBase.generated.h"

class USkeletalMeshComponent;
class UAnimMontage;
class USoundBase;
class UNiagaraSystem;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, MaxAmmo);

/**
 * Clase base para armas de fuego replicadas en red (Hitscan / Raycast con validación en servidor).
 */
UCLASS(Abstract)
class TFPEV2_API APEV2_WeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APEV2_WeaponBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==========================================
	// ACCIONES DEL JUGADOR
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void Reload();

	/** Vincula el arma a la malla del personaje propietario */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void AttachToOwnerCharacter(ACharacter* InCharacter, FName SocketName);

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	int32 GetMaxMagazineAmmo() const { return MaxMagazineAmmo; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Weapon|Events")
	FOnAmmoChanged OnAmmoChangedEvent;

protected:
	virtual void BeginPlay() override;

	// ==========================================
	// COMPONENTES DE MALLA
	// ==========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;

	// ==========================================
	// CONFIGURACIÓN Y BALANCE
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	FText WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float DamagePerShot = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float FireRate = 0.15f; // Tiempo en segundos entre disparos

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float MaxRange = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	int32 MaxMagazineAmmo = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	bool bIsAutomatic = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Sockets")
	FName MuzzleSocketName = FName(TEXT("Muzzle"));

	// ==========================================
	// EFECTOS VISUALES Y SONOROS
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UAnimMontage> FiringMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UNiagaraSystem> MuzzleFlashVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UNiagaraSystem> ImpactVFX;

	// ==========================================
	// REPLICACIÓN EN RED
	// ==========================================

	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, BlueprintReadOnly, Category = "PEV2|Weapon")
	int32 CurrentAmmo;

	UFUNCTION()
	void OnRep_CurrentAmmo();

	// RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX(FVector HitLocation);

	/** Lógica de disparo en servidor */
	void PerformServerShot();

private:
	FTimerHandle RefireTimerHandle;
	FTimerHandle ReloadTimerHandle;
	bool bIsFiring;
	bool bIsReloading;
};
