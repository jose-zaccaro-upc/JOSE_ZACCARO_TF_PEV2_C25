// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PEV2_HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, class UPEV2_HealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeath, class AController*, Killer, class AActor*, DamageCauser, const class UDamageType*, DamageType);

/**
 * Health component to manage character life and death.
 */
UCLASS( ClassGroup=(PEV2), meta=(BlueprintSpawnableComponent) )
class TFPEV2_API UPEV2_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPEV2_HealthComponent();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Health")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "PEV2|Health")
	float CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnHealthChanged OnHealthChangedEvent;

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Events")
	FOnDeath OnDeathEvent;

	bool IsDead() const { return CurrentHealth <= 0.0f; }
};
