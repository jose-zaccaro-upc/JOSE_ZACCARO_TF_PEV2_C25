// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PEV2_ProjectileBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class USoundBase;
class UNiagaraSystem;

/**
 * Proyectil balístico/explosivo replicado para armas y granadas.
 */
UCLASS()
class TFPEV2_API APEV2_ProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APEV2_ProjectileBase();

	virtual void BeginPlay() override;

	/** Inicializa y dispara el proyectil con impulso hacia una dirección */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Projectile")
	void FireInDirection(const FVector& ShootDirection, float InitialSpeed = 2000.0f);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// ==========================================
	// PROPIEDADES DE BALANCE
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	float BaseDamage = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	float DamageRadius = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	bool bIsExplosive = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	float FuseTime = 2.5f;

	// ==========================================
	// EFECTOS VISUALES Y SONOROS
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Explode();

	/** Multicast para reproducir sonido y partículas a todos los clientes */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayExplosionFX(FVector ExplosionLocation);

private:
	FTimerHandle FuseTimerHandle;
	bool bHasExploded;
};
