// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PEV2_BattleZone.generated.h"

class UStaticMeshComponent;

UCLASS()
class TFPEV2_API APEV2_BattleZone : public AActor
{
	GENERATED_BODY()
	
public:	
	APEV2_BattleZone();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PEV2|Zone")
	UStaticMeshComponent* ZoneMesh;

	/** Current radius of the zone */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentRadius, BlueprintReadOnly, Category = "PEV2|Zone")
	float CurrentRadius;

	UFUNCTION()
	void OnRep_CurrentRadius();

	/** Target radius to shrink to */
	UPROPERTY(EditDefaultsOnly, Category = "PEV2|Zone")
	float TargetRadius;

	/** Initial radius of the zone */
	UPROPERTY(EditDefaultsOnly, Category = "PEV2|Zone")
	float InitialRadius;

	/** How much damage to deal per tick when outside */
	UPROPERTY(EditDefaultsOnly, Category = "PEV2|Zone")
	float DamagePerTick;

	/** How often to apply damage (in seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "PEV2|Zone")
	float DamageTickRate;

	/** Time it takes to shrink from Initial to Target */
	UPROPERTY(EditDefaultsOnly, Category = "PEV2|Zone")
	float ShrinkDuration;

	// Timers and State
	FTimerHandle DamageTimerHandle;
	float ShrinkElapsedTime;
	bool bIsShrinking;

	void ApplyZoneDamage();
	void UpdateZoneScale();

public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Start shrinking the zone */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Zone")
	void StartShrinking();
};
