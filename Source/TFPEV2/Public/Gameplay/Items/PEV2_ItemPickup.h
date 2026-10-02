// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PEV2_InteractableInterface.h"
#include "PEV2_ItemPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class URotatingMovementComponent;
class UPointLightComponent;
class UPEV2_AbilityData;

/**
 * Actor de ítem en el suelo configurable directamente en el Editor de Unreal.
 * Permite colocar habilidades como Dash, Granada, etc. con auto-recogida y reaparición.
 */
UCLASS()
class TFPEV2_API APEV2_ItemPickup : public AActor, public IPEV2_InteractableInterface
{
	GENERATED_BODY()
	
public:	
	APEV2_ItemPickup();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

	// ==========================================
	// CONFIGURACIÓN EXPUESTA AL EDITOR DE UNREAL
	// ==========================================

	/** Habilidad otorgada por este pickup al recogerlo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Pickup")
	TObjectPtr<UPEV2_AbilityData> AbilityData;

	/** Si es true, el jugador lo recoge automáticamente al pasar por encima. Si es false, requiere pulsar 'E'. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Pickup")
	bool bAutoPickupOnOverlap = true;

	/** Si es true, el ítem reaparece tras ser recogido */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Pickup")
	bool bRespawn = true;

	/** Tiempo en segundos antes de que el ítem reaparezca en el suelo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Pickup", meta = (ClampMin = "1.0", ClampMax = "120.0", Units = "s"))
	float RespawnTime = 10.0f;

	/** Velocidad de oscilación vertical (efecto flotante) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Visuals")
	float BobbingSpeed = 2.5f;

	/** Altura máxima de la oscilación vertical en cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PEV2|Visuals")
	float BobbingHeight = 12.0f;

	// ==========================================
	// IPEV2_InteractableInterface IMPLEMENTACIÓN
	// ==========================================

	virtual bool CanInteract_Implementation(ACharacter* Interactor) const override;
	virtual void Interact_Implementation(ACharacter* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation(ACharacter* Interactor) const override;
	virtual bool IsAutoPickup_Implementation() const override { return bAutoPickupOnOverlap; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URotatingMovementComponent> RotatingMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> GlowLight;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Intenta otorgar la habilidad al personaje */
	void ProcessPickup(ACharacter* TargetCharacter);

	/** Sincroniza la desaparición del objeto y efectos en clientes */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPickedUp();

	/** Sincroniza la reaparición del objeto en clientes */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRespawned();

private:
	FTimerHandle RespawnTimerHandle;
	FVector InitialMeshRelativeLocation;
	float RunningTime;
};
