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
class APEV2_WeaponBase;

/**
 * Proyectil balístico/explosivo replicado para todas las armas y granadas.
 * Soporta caída por gravedad basada en peso, velocidad balística, empuje cinético y headshots.
 */
UCLASS()
class TFPEV2_API APEV2_ProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APEV2_ProjectileBase();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Inicializa los parámetros balísticos inyectados desde el arma disparadora */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Projectile")
	void InitializeBallistics(
		float InDamage,
		float InHeadshotMultiplier,
		float InLaunchSpeed,
		float InWeight,
		float InImpulseScale,
		bool bInIsExplosive,
		UNiagaraSystem* InImpactVFX = nullptr,
		USoundBase* InImpactSound = nullptr,
		bool bInExplodeOnImpact = false
	);

	UFUNCTION(BlueprintCallable, Category = "PEV2|Projectile")
	void SetExplodeOnImpact(bool bInExplodeOnImpact) { bExplodeOnImpact = bInExplodeOnImpact; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	bool ShouldExplodeOnImpact() const { return bExplodeOnImpact; }

	UFUNCTION(BlueprintCallable, Category = "PEV2|Projectile")
	void SetIsAbilityProjectile(bool bInIsAbility) { bIsAbilityProjectile = bInIsAbility; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	bool IsAbilityProjectile() const { return bIsAbilityProjectile; }

	UFUNCTION(BlueprintCallable, Category = "PEV2|Projectile")
	void SetFuseTime(float InFuseTime) { FuseTime = InFuseTime; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	float GetFuseTime() const { return FuseTime; }

	UFUNCTION(BlueprintCallable, Category = "PEV2|Projectile")
	void SetSourceWeapon(APEV2_WeaponBase* InWeapon) { SourceWeapon = InWeapon; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	APEV2_WeaponBase* GetSourceWeapon() const { return SourceWeapon; }

	/** Dispara el proyectil hacia una dirección con velocidad balística */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Projectile")
	void FireInDirection(const FVector& ShootDirection, float InitialSpeed = 6000.0f);

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	UStaticMeshComponent* GetProjectileMesh() const { return ProjectileMesh; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	USphereComponent* GetCollisionComp() const { return CollisionComp; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	bool IsExplosive() const { return bIsExplosive; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Projectile")
	float GetDamage() const { return BaseDamage; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// ==========================================
	// PROPIEDADES DE BALANCE Y BALÍSTICA
	// ==========================================

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	float BaseDamage = 25.0f;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	float HeadshotMultiplier = 2.0f;

	/** Peso del proyectil. Afecta directamente la caída por gravedad (ProjectileGravityScale) y el impulso cinético */
	UPROPERTY(ReplicatedUsing = OnRep_BallisticConfig, EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Ballistics")
	float ProjectileWeight = 0.15f;

	/** Velocidad inicial en cm/s */
	UPROPERTY(ReplicatedUsing = OnRep_BallisticConfig, EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Ballistics")
	float InitialLaunchSpeed = 6000.0f;

	/** Escala de fuerza cinetica transferida a objetos que simulan fisica y ragdolls */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Ballistics")
	float PhysicsImpulseScale = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	float DamageRadius = 350.0f;

	UPROPERTY(ReplicatedUsing = OnRep_BallisticConfig, EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	bool bIsExplosive = false;

	/** Si es true, detona de inmediato al chocar contra cualquier superficie o actor */
	UPROPERTY(ReplicatedUsing = OnRep_BallisticConfig, EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Damage")
	bool bExplodeOnImpact = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PEV2|Projectile")
	bool bIsAbilityProjectile = false;

	UPROPERTY(BlueprintReadOnly, Category = "PEV2|Projectile")
	TObjectPtr<APEV2_WeaponBase> SourceWeapon;

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
	void OnRep_BallisticConfig();

	void ApplyBallisticSettings();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Explode();

	/** Multicast para reproducir sonido y partículas a todos los clientes */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayExplosionFX(FVector_NetQuantize ExplosionLocation);

private:
	FTimerHandle FuseTimerHandle;
	bool bHasExploded;
};
