// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PEV2_WeaponBase.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UAnimMontage;
class UAnimInstance;
class UAnimSequenceBase;
class USoundBase;
class UNiagaraSystem;
class ACharacter;
class APEV2_ProjectileBase;

UENUM(BlueprintType)
enum class EPEV2_WeaponType : uint8
{
	Hitscan     UMETA(DisplayName = "Hitscan (Trazado de Rayos)"),
	Projectile  UMETA(DisplayName = "Proyectil Balístico")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, MaxAmmo);

/**
 * Clase base para armas de fuego replicadas en red (Hitscan con trazado de rayos o Proyectil balístico).
 * Soporta mallas esqueléticas y estáticas para máxima compatibilidad con placeholders de Fab.
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
	void StartFireWithAim(const FVector& AimStart, const FVector& AimDir);

	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void Reload();

	/** Vincula el arma a la malla del personaje propietario */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void AttachToOwnerCharacter(ACharacter* InCharacter, FName SocketName);

	/** Ajusta visibilidad de mallas para cuando la camara pasa a tercera persona en muerte */
	UFUNCTION(BlueprintCallable, Category = "PEV2|Weapon")
	void SetThirdPersonDeathView(bool bInDeathView);

	virtual void OnRep_Owner() override;

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	int32 GetMaxMagazineAmmo() const { return MaxMagazineAmmo; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	UStaticMeshComponent* GetFirstPersonStaticMesh() const { return FirstPersonStaticMesh; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	UStaticMeshComponent* GetThirdPersonStaticMesh() const { return ThirdPersonStaticMesh; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Animation")
	TSubclassOf<UAnimInstance> GetThirdPersonAnimInstanceClass() const { return ThirdPersonAnimInstanceClass; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Animation")
	TSubclassOf<UAnimInstance> GetFirstPersonAnimInstanceClass() const { return FirstPersonAnimInstanceClass; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	FText GetWeaponName() const { return WeaponName; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	class UTexture2D* GetWeaponIcon() const { return WeaponIcon; }

	UFUNCTION(BlueprintPure, Category = "PEV2|Weapon")
	bool IsMeleeWeapon() const { return bIsMelee; }

	UPROPERTY(BlueprintAssignable, Category = "PEV2|Weapon|Events")
	FOnAmmoChanged OnAmmoChangedEvent;

protected:
	virtual void BeginPlay() override;

	// ==========================================
	// COMPONENTES DE MALLA (Skeletal y Static para Fab)
	// ==========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;

	/** Malla estática opcional en primera persona para modelos de Fab */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FirstPersonStaticMesh;

	/** Malla estática opcional en tercera persona para modelos de Fab */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ThirdPersonStaticMesh;

	// ==========================================
	// CONFIGURACIÓN Y BALANCE
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	EPEV2_WeaponType WeaponType = EPEV2_WeaponType::Projectile;

	/** Si es true, el arma es cuerpo a cuerpo (ej. Cuchillo de combate) y ataca por barrido cercano */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	bool bIsMelee = false;

	/** Rango máximo de impacto para ataque cuerpo a cuerpo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance", meta = (EditCondition = "bIsMelee"))
	float MeleeRange = 175.0f;

	/** Radio de la esfera de impacto para ataque cuerpo a cuerpo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance", meta = (EditCondition = "bIsMelee"))
	float MeleeRadius = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	FText WeaponName;

	/** Icono representativo del arma para la interfaz y visor de arsenal */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|UI")
	TObjectPtr<class UTexture2D> WeaponIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float DamagePerShot = 25.0f;

	/** Multiplicador de daño crítico al impactar la cabeza (Bone Name contiene 'head') */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float HeadshotMultiplier = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float FireRate = 0.15f; // Tiempo en segundos entre disparos

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float MaxRange = 10000.0f;

	/** Ángulo de dispersión en grados (0 = precisión perfecta) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float SpreadAngle = 0.5f;

	/** Cantidad de proyectiles/rayos por disparo (1 para rifle/pistola, 6-8 para escopeta) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance", meta = (ClampMin = "1"))
	int32 PelletsPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	int32 MaxMagazineAmmo = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	bool bIsAutomatic = true;

	// ==========================================
	// CONFIGURACIÓN DE PROYECTILES BALÍSTICOS
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Projectile")
	TSubclassOf<APEV2_ProjectileBase> ProjectileClass;

	/** Velocidad inicial en cm/s (ej. 6000 para balas rapidas, 2500 para granadas) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Projectile")
	float ProjectileLaunchSpeed = 6000.0f;

	/** Peso del proyectil (controla la caida por gravedad y el empuje fisico de impacto) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Projectile")
	float ProjectileWeight = 0.15f;

	/** Multiplicador de impulso fisico transferido al impactar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Projectile")
	float ProjectileImpulseScale = 15.0f;

	/** Si es true, el proyectil generado detona con daño radial (ej. lanzagranadas) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Projectile")
	bool bSpawnExplosiveProjectile = false;

	/** Si es true, el proyectil detona en el primer impacto con cualquier superficie (ej. cohete). Si es false, rebota en superficies y explota por tiempo (espoleta) o al chocar contra un enemigo (ej. lanzagranadas). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Projectile", meta = (EditCondition = "bSpawnExplosiveProjectile"))
	bool bExplodeOnImpact = false;

	/** Tiempo en segundos antes de detonar automáticamente tras ser disparado si rebota y no impacta a un enemigo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Projectile", meta = (EditCondition = "bSpawnExplosiveProjectile && !bExplodeOnImpact"))
	float ProjectileFuseTime = 2.0f;

	// ==========================================
	// SOCKETS Y ENSAMBLAJE
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Sockets")
	FName MuzzleSocketName = FName(TEXT("Muzzle"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Sockets")
	FName WeaponAttachSocket = FName(TEXT("HandGrip_R"));

	/** Si es true, compensa automáticamente los 90 grados de rotación de mallas FBX importadas (ej. ZC-57 de Fab) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Sockets")
	bool bCompensateImportRotation = false;

	/** Desplazamiento relativo para la malla de tercera persona en el socket del personaje */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Sockets")
	FVector ThirdPersonMeshOffset = FVector::ZeroVector;

	/** Rotación adicional para la malla de tercera persona en el socket del personaje */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Sockets")
	FRotator ThirdPersonMeshRotation = FRotator::ZeroRotator;

	/** Escala para la malla de tercera persona en el socket del personaje */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Sockets")
	FVector ThirdPersonMeshScale = FVector(1.0f, 1.0f, 1.0f);

	/** Desplazamiento relativo del arma respecto a la FirstPersonCamera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Camera")
	FVector FirstPersonWeaponOffset = FVector(28.0f, 13.0f, -15.0f);

	/** Rotación relativa del arma respecto a la FirstPersonCamera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Camera")
	FRotator FirstPersonWeaponRotation = FRotator(-1.5f, -1.0f, 0.0f);

	/** Escala del arma en primera persona */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PEV2|Camera")
	FVector FirstPersonWeaponScale = FVector(1.0f, 1.0f, 1.0f);

	// ==========================================
	// ANIMACIÓN DEL PERSONAJE
	// ==========================================

	/** Anim Blueprint para los brazos en primera persona al sostener esta arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** Anim Blueprint para el cuerpo en tercera persona al sostener esta arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	// ==========================================
	// EFECTOS VISUALES Y SONOROS
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UAnimMontage> FiringMontage;

	/** Secuencia de animación de disparo o golpe cuerpo a cuerpo como alternativa a montaje */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UAnimSequenceBase> FiringAnimSequence;

	/** Secuencias de ataque cuerpo a cuerpo encadenadas (combos) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TArray<TObjectPtr<UAnimSequenceBase>> MeleeAttackSequences;

	int32 CurrentMeleeAnimIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UAnimSequenceBase> ReloadAnimSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Balance")
	float ReloadDuration = 1.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UNiagaraSystem> MuzzleFlashVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UNiagaraSystem> TracerVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PEV2|Effects")
	TObjectPtr<UNiagaraSystem> ImpactVFX;

	// ==========================================
	// REPLICACIÓN EN RED
	// ==========================================

	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, BlueprintReadOnly, Category = "PEV2|Weapon")
	int32 CurrentAmmo;

	UFUNCTION()
	void OnRep_CurrentAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_IsReloading, BlueprintReadOnly, Category = "PEV2|Weapon")
	bool bIsReloading;

	UFUNCTION()
	void OnRep_IsReloading();

	// RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartFire(FVector_NetQuantize AimStart, FVector_NetQuantizeNormal AimDir);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX(FVector_NetQuantize HitLocation, bool bHit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReloadFX();

	/** Lógica de disparo en servidor */
	void PerformServerShot();

	/** Obtiene la ubicación del cañón según la malla visible activa */
	FVector GetMuzzleLocation() const;

private:
	FTimerHandle RefireTimerHandle;
	FTimerHandle ReloadTimerHandle;
	bool bIsFiring;

	FVector LastAimStart;
	FVector LastAimDirection;
};
