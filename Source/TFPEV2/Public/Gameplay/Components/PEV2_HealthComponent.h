// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PEV2_HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, class UPEV2_HealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnShieldChanged, class UPEV2_HealthComponent*, HealthComp, float, Shield, float, ShieldDelta, float, MaxShield);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeath, class AController*, Killer, class AActor*, DamageCauser, const class UDamageType*, DamageType);

/**
 * Health component to manage character life, shield, and death replication.
 */
UCLASS( ClassGroup=(PEV2), meta=(BlueprintSpawnableComponent) )
class TFPEV2_API UPEV2_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPEV2_HealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Agrega escudo temporal al personaje (Autoridad) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PEV2|Health")
	void AddShield(float Amount, float Duration = 0.0f);

	/** Restaura salud y escudo a sus valores iniciales (Autoridad) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PEV2|Health")
	void ResetHealthAndShield();

	UFUNCTION(BlueprintPure, Category = "PEV2|Health")
	bool IsDead() const { return CurrentHealth <= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Health")
	float GetHealthPercent() const { return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Health")
	float GetShieldPercent() const { return (MaxShield > 0.0f) ? (CurrentShield / MaxShield) : 0.0f; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Health")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "PEV2|Health")
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Health")
	float MaxShield;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentShield, BlueprintReadOnly, Category = "PEV2|Health")
	float CurrentShield;

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_CurrentShield();

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnHealthChanged OnHealthChangedEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnShieldChanged OnShieldChangedEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnDeath OnDeathEvent;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void HandleTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	UFUNCTION()
	void HandleTakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const struct FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

private:
	FTimerHandle ShieldExpireTimerHandle;
};
