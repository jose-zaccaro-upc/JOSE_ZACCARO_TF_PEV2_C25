#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "Gameplay/Weapons/PEV2_ProjectileBase.h"
#include "TFPEV2Character.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

APEV2_WeaponBase::APEV2_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);
	FirstPersonMesh->SetCastShadow(false);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);

	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);
	ThirdPersonMesh->SetOwnerNoSee(true);
	ThirdPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);

	FirstPersonStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonStaticMesh"));
	FirstPersonStaticMesh->SetupAttachment(RootComponent);
	FirstPersonStaticMesh->SetCastShadow(false);
	FirstPersonStaticMesh->SetOnlyOwnerSee(true);
	FirstPersonStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonStaticMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);

	ThirdPersonStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThirdPersonStaticMesh"));
	ThirdPersonStaticMesh->SetupAttachment(RootComponent);
	ThirdPersonStaticMesh->SetOwnerNoSee(true);
	ThirdPersonStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ThirdPersonStaticMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);

	CurrentAmmo = 30;
	MaxMagazineAmmo = 30;
	DamagePerShot = 25.0f;
	FireRate = 0.15f;
	MaxRange = 10000.0f;
	bIsAutomatic = true;

	WeaponType = EPEV2_WeaponType::Projectile;
	ProjectileLaunchSpeed = 6000.0f;
	ProjectileWeight = 0.15f;
	ProjectileImpulseScale = 15.0f;
	bSpawnExplosiveProjectile = false;
	bIsMelee = false;
	MeleeRange = 175.0f;
	MeleeRadius = 35.0f;

	bIsFiring = false;
	bIsReloading = false;
	ReloadDuration = 1.8f;

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultReloadSound(TEXT("/Game/PEV2/Audio/S_WeaponReload.S_WeaponReload"));
	if (DefaultReloadSound.Succeeded())
	{
		ReloadSound = DefaultReloadSound.Object;
	}

	LastAimStart = FVector::ZeroVector;
	LastAimDirection = FVector::ZeroVector;
}

void APEV2_WeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APEV2_WeaponBase, CurrentAmmo);
	DOREPLIFETIME(APEV2_WeaponBase, bIsReloading);
}

void APEV2_WeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentAmmo = MaxMagazineAmmo;
	}

	const FString WName = GetName().ToLower();
	const bool bIsPistol = WName.Contains(TEXT("pistol"));

	// 1. Fallback de audio de recarga y disparo del Starter de Unreal
	if (!ReloadSound)
	{
		ReloadSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/PEV2/Audio/S_WeaponReload.S_WeaponReload"));
	}

	if (!FireSound)
	{
		FireSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/PEV2/Gameplay/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02"));
	}

	// 2. Fallback de AnimInstance de tercera persona (postura y locomoción del personaje con arma)
	if (!ThirdPersonAnimInstanceClass)
	{
		if (bIsMelee)
		{
			ThirdPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
		}
		else if (bIsPistol)
		{
			ThirdPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Pistol.ABP_TP_Pistol_C"));
		}
		else
		{
			ThirdPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C"));
		}
	}

	// 3. Fallback de AnimInstance de primera persona
	if (!FirstPersonAnimInstanceClass && !bIsMelee)
	{
		if (bIsPistol)
		{
			FirstPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Variant_Shooter/Anims/ABP_FP_Pistol.ABP_FP_Pistol_C"));
		}
		else
		{
			FirstPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Variant_Shooter/Anims/ABP_FP_Weapon.ABP_FP_Weapon_C"));
		}
	}

	// 4. Fallback de animación de disparo (Montaje o Secuencia del Starter)
	if (!FiringMontage && !FiringAnimSequence)
	{
		if (bIsMelee)
		{
			FiringAnimSequence = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01"));
		}
		else if (bIsPistol)
		{
			FiringMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage.MM_Pistol_Fire_Montage"));
			if (!FiringMontage)
			{
				FiringAnimSequence = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire.MM_Pistol_Fire"));
			}
		}
		else
		{
			FiringMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_Fire_Montage.MM_Rifle_Fire_Montage"));
			if (!FiringMontage)
			{
				FiringAnimSequence = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_Fire.MM_Rifle_Fire"));
			}
		}
	}

	// 5. Fallback de animación de recarga del Starter
	if (!ReloadMontage && !ReloadAnimSequence && !bIsMelee)
	{
		if (bIsPistol)
		{
			ReloadAnimSequence = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Reload.MM_Pistol_Reload"));
		}
		else
		{
			ReloadAnimSequence = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_Reload.MM_Rifle_Reload"));
		}
	}
}

void APEV2_WeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (ACharacter* CharOwner = Cast<ACharacter>(GetOwner()))
	{
		AttachToOwnerCharacter(CharOwner, WeaponAttachSocket);
	}
}

void APEV2_WeaponBase::SetThirdPersonDeathView(bool bInDeathView)
{
	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
	const bool bIsLocalOwner = CharOwner ? CharOwner->IsLocallyControlled() : false;

	if (bInDeathView)
	{
		if (FirstPersonMesh) FirstPersonMesh->SetVisibility(false, true);
		if (FirstPersonStaticMesh) FirstPersonStaticMesh->SetVisibility(false, true);

		if (ThirdPersonMesh && ThirdPersonMesh->GetSkeletalMeshAsset())
		{
			ThirdPersonMesh->SetOwnerNoSee(false);
			ThirdPersonMesh->SetVisibility(true, true);
		}
		if (ThirdPersonStaticMesh && ThirdPersonStaticMesh->GetStaticMesh())
		{
			ThirdPersonStaticMesh->SetOwnerNoSee(false);
			ThirdPersonStaticMesh->SetVisibility(true, true);
		}
	}
	else
	{
		if (FirstPersonMesh) FirstPersonMesh->SetVisibility(bIsLocalOwner && FirstPersonMesh->GetSkeletalMeshAsset() != nullptr, true);
		if (FirstPersonStaticMesh) FirstPersonStaticMesh->SetVisibility(bIsLocalOwner && FirstPersonStaticMesh->GetStaticMesh() != nullptr, true);

		if (ThirdPersonMesh)
		{
			ThirdPersonMesh->SetOwnerNoSee(bIsLocalOwner);
			ThirdPersonMesh->SetVisibility(!bIsLocalOwner && ThirdPersonMesh->GetSkeletalMeshAsset() != nullptr, true);
		}
		if (ThirdPersonStaticMesh)
		{
			ThirdPersonStaticMesh->SetOwnerNoSee(bIsLocalOwner);
			ThirdPersonStaticMesh->SetVisibility(!bIsLocalOwner && ThirdPersonStaticMesh->GetStaticMesh() != nullptr, true);
		}
	}
}

void APEV2_WeaponBase::AttachToOwnerCharacter(ACharacter* InCharacter, FName SocketName)
{
	if (!InCharacter)
	{
		return;
	}

	SetOwner(InCharacter);
	SetInstigator(InCharacter);

	AttachToActor(InCharacter, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));

	// Sincronización automática de mallas si el Blueprint solo asignó una de las dos
	if (FirstPersonMesh && ThirdPersonMesh)
	{
		if (FirstPersonMesh->GetSkeletalMeshAsset() && !ThirdPersonMesh->GetSkeletalMeshAsset())
		{
			ThirdPersonMesh->SetSkeletalMeshAsset(FirstPersonMesh->GetSkeletalMeshAsset());
		}
		else if (ThirdPersonMesh->GetSkeletalMeshAsset() && !FirstPersonMesh->GetSkeletalMeshAsset())
		{
			FirstPersonMesh->SetSkeletalMeshAsset(ThirdPersonMesh->GetSkeletalMeshAsset());
		}
	}

	if (FirstPersonStaticMesh && ThirdPersonStaticMesh)
	{
		if (FirstPersonStaticMesh->GetStaticMesh() && !ThirdPersonStaticMesh->GetStaticMesh())
		{
			ThirdPersonStaticMesh->SetStaticMesh(FirstPersonStaticMesh->GetStaticMesh());
		}
		else if (ThirdPersonStaticMesh->GetStaticMesh() && !FirstPersonStaticMesh->GetStaticMesh())
		{
			FirstPersonStaticMesh->SetStaticMesh(ThirdPersonStaticMesh->GetStaticMesh());
		}
	}

	// Asegurar que las mallas del arma nunca bloqueen disparos o cámara
	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FirstPersonMesh->SetOnlyOwnerSee(true);
	}
	if (ThirdPersonMesh)
	{
		ThirdPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThirdPersonMesh->SetOwnerNoSee(true);
	}
	if (FirstPersonStaticMesh)
	{
		FirstPersonStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FirstPersonStaticMesh->SetOnlyOwnerSee(true);
	}
	if (ThirdPersonStaticMesh)
	{
		ThirdPersonStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThirdPersonStaticMesh->SetOwnerNoSee(true);
	}

	const bool bIsLocalOwner = InCharacter->IsLocallyControlled();

	// Detectar si la malla tiene rotación de importación de 90° (como la ZC-57 de Fab)
	FString MeshName;
	if (ThirdPersonMesh && ThirdPersonMesh->GetSkeletalMeshAsset())
	{
		MeshName = ThirdPersonMesh->GetSkeletalMeshAsset()->GetName().ToLower();
	}
	else if (FirstPersonMesh && FirstPersonMesh->GetSkeletalMeshAsset())
	{
		MeshName = FirstPersonMesh->GetSkeletalMeshAsset()->GetName().ToLower();
	}

	const bool bIsRotatedImport = MeshName.Contains(TEXT("zc57")) || MeshName.Contains(TEXT("zc-57")) || bCompensateImportRotation;

	// Vincular mallas de tercera persona al cuerpo del personaje (HandGrip_R)
	if (InCharacter->GetMesh())
	{
		FRotator TPFinalRot = ThirdPersonMeshRotation;
		if (bIsRotatedImport)
		{
			TPFinalRot = FRotator(0.0f, -90.0f, 0.0f);
		}
		else if (bIsMelee && TPFinalRot.IsZero())
		{
			TPFinalRot = FRotator(-90.0f, 0.0f, 0.0f);
		}

		if (ThirdPersonMesh && ThirdPersonMesh->GetSkeletalMeshAsset())
		{
			ThirdPersonMesh->AttachToComponent(InCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			ThirdPersonMesh->SetRelativeRotation(TPFinalRot);
			ThirdPersonMesh->SetOwnerNoSee(bIsLocalOwner);
			ThirdPersonMesh->SetVisibility(!bIsLocalOwner, true);
		}
		else if (ThirdPersonMesh)
		{
			ThirdPersonMesh->SetVisibility(false, true);
		}

		if (ThirdPersonStaticMesh && ThirdPersonStaticMesh->GetStaticMesh())
		{
			ThirdPersonStaticMesh->AttachToComponent(InCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			ThirdPersonStaticMesh->SetRelativeRotation(TPFinalRot);
			ThirdPersonStaticMesh->SetOwnerNoSee(bIsLocalOwner);
			ThirdPersonStaticMesh->SetVisibility(!bIsLocalOwner, true);
		}
		else if (ThirdPersonStaticMesh)
		{
			ThirdPersonStaticMesh->SetVisibility(false, true);
		}
	}

	// Vincular mallas de primera persona a FirstPersonCameraComponent
	// Esto sitúa la cámara exactamente por sobre el mango del arma, permitiendo ver el costado
	// superior izquierdo del arma sin que los brazos ni el cuerpo obstruyan la visión ni distorsionen vértices.
	UCameraComponent* Cam = nullptr;
	if (ATFPEV2Character* TFChar = Cast<ATFPEV2Character>(InCharacter))
	{
		Cam = TFChar->GetFirstPersonCameraComponent();
		if (TFChar->GetFirstPersonMesh())
		{
			TFChar->GetFirstPersonMesh()->SetVisibility(false, true);
		}
	}

	if (Cam && bIsLocalOwner)
	{
		const FRotator FPFinalRot = bIsRotatedImport ? FRotator(-1.5f, -87.0f, -2.0f) : FirstPersonWeaponRotation;

		if (FirstPersonMesh && FirstPersonMesh->GetSkeletalMeshAsset())
		{
			FirstPersonMesh->AttachToComponent(Cam, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			FirstPersonMesh->SetRelativeLocationAndRotation(FirstPersonWeaponOffset, FPFinalRot);
			FirstPersonMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
			FirstPersonMesh->SetOnlyOwnerSee(true);
			FirstPersonMesh->SetVisibility(true, true);
			FirstPersonMesh->SetAnimInstanceClass(nullptr);
		}
		else if (FirstPersonMesh)
		{
			FirstPersonMesh->SetVisibility(false, true);
		}

		if (FirstPersonStaticMesh && FirstPersonStaticMesh->GetStaticMesh())
		{
			FirstPersonStaticMesh->AttachToComponent(Cam, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			FirstPersonStaticMesh->SetRelativeLocationAndRotation(FirstPersonWeaponOffset, FPFinalRot);
			FirstPersonStaticMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
			FirstPersonStaticMesh->SetOnlyOwnerSee(true);
			FirstPersonStaticMesh->SetVisibility(true, true);
		}
		else if (FirstPersonStaticMesh)
		{
			FirstPersonStaticMesh->SetVisibility(false, true);
		}
	}
	else
	{
		// Si no es el dueño local, u omitimos cámara, ocultar terminantemente mallas 1P
		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetVisibility(false, true);
		}
		if (FirstPersonStaticMesh)
		{
			FirstPersonStaticMesh->SetVisibility(false, true);
		}
	}

	// Asignar AnimInstance de tercera persona con soporte prioritario para cuerpo a cuerpo
	if (bIsMelee)
	{
		if (!ThirdPersonAnimInstanceClass ||
			ThirdPersonAnimInstanceClass->GetName().Contains(TEXT("Pistol")) ||
			ThirdPersonAnimInstanceClass->GetName().Contains(TEXT("Rifle")))
		{
			ThirdPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
		}
	}
	else if (!ThirdPersonAnimInstanceClass)
	{
		const FString WName = GetName().ToLower();
		if (WName.Contains(TEXT("pistol")))
		{
			ThirdPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Pistol.ABP_TP_Pistol_C"));
		}
		else
		{
			ThirdPersonAnimInstanceClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C"));
		}
	}

	if (ThirdPersonAnimInstanceClass && InCharacter->GetMesh())
	{
		InCharacter->GetMesh()->SetAnimInstanceClass(ThirdPersonAnimInstanceClass);
	}
}

FVector APEV2_WeaponBase::GetMuzzleLocation() const
{
	static const TArray<FName> CandidateSockets = {
		MuzzleSocketName,
		FName(TEXT("Socket_Barrel_end")),
		FName(TEXT("Socket_Barrel")),
		FName(TEXT("Barrel")),
		FName(TEXT("Muzzle")),
		FName(TEXT("b_gun_barrel")),
		FName(TEXT("b_gun_muzzle")),
		FName(TEXT("muzzle_flash")),
		FName(TEXT("Root_Grip")),
		FName(TEXT("HandGrip_R"))
	};

	// 1. Malla de primera persona del jugador local (si está visible)
	if (FirstPersonMesh && FirstPersonMesh->GetSkeletalMeshAsset() && FirstPersonMesh->IsVisible())
	{
		for (const FName& SName : CandidateSockets)
		{
			if (FirstPersonMesh->DoesSocketExist(SName))
			{
				return FirstPersonMesh->GetSocketLocation(SName);
			}
		}
		return FirstPersonMesh->GetComponentLocation() + (FirstPersonMesh->GetForwardVector() * 45.0f);
	}

	// 2. Malla de tercera persona (para remotos o si 1P no tiene socket)
	if (ThirdPersonMesh && ThirdPersonMesh->GetSkeletalMeshAsset())
	{
		for (const FName& SName : CandidateSockets)
		{
			if (ThirdPersonMesh->DoesSocketExist(SName))
			{
				return ThirdPersonMesh->GetSocketLocation(SName);
			}
		}
		return ThirdPersonMesh->GetComponentLocation() + (ThirdPersonMesh->GetForwardVector() * 45.0f);
	}

	// 3. Malla estática de primera persona
	if (FirstPersonStaticMesh && FirstPersonStaticMesh->GetStaticMesh() && FirstPersonStaticMesh->IsVisible())
	{
		for (const FName& SName : CandidateSockets)
		{
			if (FirstPersonStaticMesh->DoesSocketExist(SName))
			{
				return FirstPersonStaticMesh->GetSocketLocation(SName);
			}
		}
		return FirstPersonStaticMesh->GetComponentLocation() + (FirstPersonStaticMesh->GetForwardVector() * 45.0f);
	}

	// 4. Malla estática de tercera persona
	if (ThirdPersonStaticMesh && ThirdPersonStaticMesh->GetStaticMesh())
	{
		for (const FName& SName : CandidateSockets)
		{
			if (ThirdPersonStaticMesh->DoesSocketExist(SName))
			{
				return ThirdPersonStaticMesh->GetSocketLocation(SName);
			}
		}
		return ThirdPersonStaticMesh->GetComponentLocation() + (ThirdPersonStaticMesh->GetForwardVector() * 45.0f);
	}

	// 5. Fallback seguro: socket de la mano del personaje o frente de la vista
	if (ACharacter* CharOwner = Cast<ACharacter>(GetOwner()))
	{
		if (CharOwner->GetMesh() && CharOwner->GetMesh()->DoesSocketExist(FName(TEXT("HandGrip_R"))))
		{
			return CharOwner->GetMesh()->GetSocketLocation(FName(TEXT("HandGrip_R"))) + (CharOwner->GetActorForwardVector() * 45.0f);
		}
		return CharOwner->GetPawnViewLocation() + (CharOwner->GetActorForwardVector() * 40.0f);
	}

	return GetActorLocation() + (GetActorForwardVector() * 50.0f);
}

void APEV2_WeaponBase::StartFire()
{
	if (bIsReloading)
	{
		return;
	}

	if (!bIsMelee && CurrentAmmo <= 0)
	{
		Reload();
		return;
	}

	FVector AimStart = GetActorLocation();
	FVector AimDir = GetActorForwardVector();

	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		FRotator ViewRot;
		Char->GetActorEyesViewPoint(AimStart, ViewRot);
		AimDir = ViewRot.Vector();

		// Si es el jugador local en primera persona, utilizar la posición y rotación exacta de la cámara
		if (Char->IsLocallyControlled())
		{
			if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
			{
				if (PC->PlayerCameraManager)
				{
					AimStart = PC->PlayerCameraManager->GetCameraLocation();
					AimDir = PC->PlayerCameraManager->GetCameraRotation().Vector();
				}
			}
		}
	}

	StartFireWithAim(AimStart, AimDir);
}

void APEV2_WeaponBase::StartFireWithAim(const FVector& AimStart, const FVector& AimDir)
{
	if (HasAuthority())
	{
		Server_StartFire_Implementation(AimStart, AimDir);
	}
	else
	{
		Server_StartFire(AimStart, AimDir);
	}
}

bool APEV2_WeaponBase::Server_StartFire_Validate(FVector_NetQuantize AimStart, FVector_NetQuantizeNormal AimDir)
{
	return true;
}

void APEV2_WeaponBase::Server_StartFire_Implementation(FVector_NetQuantize AimStart, FVector_NetQuantizeNormal AimDir)
{
	if (bIsReloading)
	{
		return;
	}

	if (!bIsMelee && CurrentAmmo <= 0)
	{
		Reload();
		return;
	}

	bIsFiring = true;
	LastAimStart = AimStart;
	LastAimDirection = AimDir;

	PerformServerShot();

	if (bIsAutomatic && CurrentAmmo > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimerHandle, this, &APEV2_WeaponBase::PerformServerShot, FireRate, true);
	}
}

void APEV2_WeaponBase::StopFire()
{
	if (HasAuthority())
	{
		Server_StopFire_Implementation();
	}
	else
	{
		Server_StopFire();
	}
}

bool APEV2_WeaponBase::Server_StopFire_Validate()
{
	return true;
}

void APEV2_WeaponBase::Server_StopFire_Implementation()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(RefireTimerHandle);
}

void APEV2_WeaponBase::Reload()
{
	if (bIsReloading || CurrentAmmo >= MaxMagazineAmmo || bIsMelee)
	{
		return;
	}

	if (HasAuthority())
	{
		Server_Reload_Implementation();
	}
	else
	{
		bIsReloading = true;
		Server_Reload();
	}
}

bool APEV2_WeaponBase::Server_Reload_Validate()
{
	return true;
}

void APEV2_WeaponBase::Server_Reload_Implementation()
{
	if (bIsReloading || CurrentAmmo >= MaxMagazineAmmo || bIsMelee)
	{
		return;
	}

	bIsReloading = true;
	Server_StopFire();

	Multicast_PlayReloadFX();

	// Tiempo de recarga simulado
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, [this]()
	{
		CurrentAmmo = MaxMagazineAmmo;
		bIsReloading = false;
		OnRep_CurrentAmmo();
		OnRep_IsReloading();
	}, ReloadDuration, false);
}

void APEV2_WeaponBase::PerformServerShot()
{
	if (!bIsMelee && CurrentAmmo <= 0)
	{
		Server_StopFire();
		return;
	}

	if (!bIsMelee)
	{
		CurrentAmmo--;
		OnRep_CurrentAmmo();
	}

	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (!CharacterOwner)
	{
		return;
	}

	FVector AimStart;
	FVector AimDir;

	// Actualizar dirección y punto de partida de apuntado en tiempo real en cada disparo de la ráfaga
	if (CharacterOwner->IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController()))
		{
			if (PC->PlayerCameraManager)
			{
				AimStart = PC->PlayerCameraManager->GetCameraLocation();
				AimDir = PC->PlayerCameraManager->GetCameraRotation().Vector();
			}
			else
			{
				AimStart = CharacterOwner->GetPawnViewLocation();
				AimDir = CharacterOwner->GetBaseAimRotation().Vector();
			}
		}
		else
		{
			AimStart = CharacterOwner->GetPawnViewLocation();
			AimDir = CharacterOwner->GetBaseAimRotation().Vector();
		}
	}
	else
	{
		// En el servidor para clientes remotos (o bots), leer la posición y rotación sincronizada del Pawn en tiempo real
		AimStart = CharacterOwner->GetPawnViewLocation();
		AimDir = CharacterOwner->GetBaseAimRotation().Vector();
	}

	LastAimStart = AimStart;
	LastAimDirection = AimDir;

	// 1. ATAQUE CUERPO A CUERPO (Cuchillo de combate)
	if (bIsMelee)
	{
		FHitResult MeleeHit;
		const FVector MeleeStart = CharacterOwner->GetPawnViewLocation();
		const FVector MeleeEnd = MeleeStart + (AimDir * MeleeRange);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(CharacterOwner);
		QueryParams.bTraceComplex = true;

		FCollisionShape SweepShape = FCollisionShape::MakeSphere(MeleeRadius);
		bool bHit = GetWorld()->SweepSingleByChannel(MeleeHit, MeleeStart, MeleeEnd, FQuat::Identity, ECC_Visibility, SweepShape, QueryParams);

		if (!bHit)
		{
			bHit = GetWorld()->LineTraceSingleByChannel(MeleeHit, MeleeStart, MeleeEnd, ECC_Visibility, QueryParams);
		}

		if (bHit && MeleeHit.GetActor())
		{
			AActor* HitActor = MeleeHit.GetActor();
			if (HitActor->GetOwner() && HitActor->GetOwner()->IsA<ACharacter>())
			{
				HitActor = HitActor->GetOwner();
			}
			else if (HitActor->GetAttachParentActor() && HitActor->GetAttachParentActor()->IsA<ACharacter>())
			{
				HitActor = HitActor->GetAttachParentActor();
			}

			// Aplicar daño del golpe con el cuchillo
			UGameplayStatics::ApplyPointDamage(
				HitActor,
				DamagePerShot,
				AimDir,
				MeleeHit,
				CharacterOwner->GetController(),
				this,
				UDamageType::StaticClass()
			);

			if (MeleeHit.GetComponent() && MeleeHit.GetComponent()->IsSimulatingPhysics())
			{
				MeleeHit.GetComponent()->AddImpulseAtLocation(AimDir * ProjectileImpulseScale * 500.0f, MeleeHit.ImpactPoint);
			}
		}

		Multicast_PlayFireFX(bHit ? MeleeHit.ImpactPoint : MeleeEnd, bHit);
		return;
	}

	// 2. DISPARO BALÍSTICO DE PROYECTILES PARA TODAS LAS ARMAS
	// Calcular punto de convergencia mediante traza de línea de visión de la cámara
	FHitResult AimSightHit;
	const FVector SightEnd = AimStart + (AimDir * MaxRange);
	FCollisionQueryParams SightParams;
	SightParams.AddIgnoredActor(this);
	SightParams.AddIgnoredActor(CharacterOwner);
	SightParams.AddIgnoredActors(CharacterOwner->Children);

	bool bSightHit = GetWorld()->LineTraceSingleByChannel(AimSightHit, AimStart, SightEnd, ECC_Visibility, SightParams);
	const FVector TargetAimPoint = bSightHit ? AimSightHit.ImpactPoint : SightEnd;

	// Origen de lanzamiento: Boca del cañón del arma (Muzzle)
	const FVector MuzzleLoc = GetMuzzleLocation();
	FVector ShootDir = (TargetAimPoint - MuzzleLoc).GetSafeNormal();

	// Prevención de convergencia anómala: si el ángulo entre el vector de disparo y la mira supera 45 grados
	// (por ejemplo, si el trazado impactó un obstáculo inmediatamente adyacente a la cámara o detrás del cañón),
	// disparar recto en la dirección de la mira para no desviar la bala lateralmente.
	if ((ShootDir | AimDir) < 0.707f)
	{
		ShootDir = AimDir;
	}

	const FVector SafeSpawnLoc = MuzzleLoc + (ShootDir * 15.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CharacterOwner;
	SpawnParams.Instigator = CharacterOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* EffectiveProjClass = ProjectileClass ? ProjectileClass.Get() : APEV2_ProjectileBase::StaticClass();

	const int32 NumPellets = FMath::Max(PelletsPerShot, 1);
	for (int32 i = 0; i < NumPellets; ++i)
	{
		FVector BaseShootDir = ShootDir;

		if (SpreadAngle > 0.0f)
		{
			const float HalfRad = FMath::DegreesToRadians(SpreadAngle);
			BaseShootDir = FMath::VRandCone(BaseShootDir, HalfRad);
		}

		const FRotator LaunchRot = BaseShootDir.Rotation();
		if (APEV2_ProjectileBase* SpawnedProj = GetWorld()->SpawnActor<APEV2_ProjectileBase>(EffectiveProjClass, SafeSpawnLoc, LaunchRot, SpawnParams))
		{
			if (SpawnedProj->GetCollisionComp())
			{
				SpawnedProj->GetCollisionComp()->IgnoreActorWhenMoving(CharacterOwner, true);
				SpawnedProj->GetCollisionComp()->IgnoreActorWhenMoving(this, true);
			}

			// Inyectar estadísticas de balística del arma al proyectil
			const bool bExplosive = bSpawnExplosiveProjectile || SpawnedProj->IsExplosive();
			const bool bShouldExplodeOnImpact = bSpawnExplosiveProjectile ? bExplodeOnImpact : SpawnedProj->ShouldExplodeOnImpact();

			SpawnedProj->SetSourceWeapon(this);
			if (bSpawnExplosiveProjectile)
			{
				SpawnedProj->SetFuseTime(ProjectileFuseTime);
			}

			SpawnedProj->InitializeBallistics(
				DamagePerShot,
				HeadshotMultiplier,
				ProjectileLaunchSpeed,
				ProjectileWeight,
				ProjectileImpulseScale,
				bExplosive,
				ImpactVFX,
				nullptr,
				bShouldExplodeOnImpact
			);

			SpawnedProj->FireInDirection(BaseShootDir, ProjectileLaunchSpeed);
		}
	}

	Multicast_PlayFireFX(TargetAimPoint, bSightHit);

	// Si se agotaron las balas de la recámara tras este disparo, disparar recarga automática inmediatamente
	if (!bIsMelee && CurrentAmmo <= 0)
	{
		Server_StopFire();
		Reload();
	}
}

void APEV2_WeaponBase::Multicast_PlayFireFX_Implementation(FVector_NetQuantize HitLocation, bool bHit)
{
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());

	if (FiringMontage)
	{
		if (CharOwner)
		{
			CharOwner->PlayAnimMontage(FiringMontage);
		}
	}
	else if (MeleeAttackSequences.Num() > 0)
	{
		if (CharOwner && CharOwner->GetMesh())
		{
			if (UAnimInstance* AnimInst = CharOwner->GetMesh()->GetAnimInstance())
			{
				UAnimSequenceBase* AnimToPlay = MeleeAttackSequences[CurrentMeleeAnimIndex % MeleeAttackSequences.Num()];
				CurrentMeleeAnimIndex++;
				if (AnimToPlay)
				{
					AnimInst->PlaySlotAnimationAsDynamicMontage(AnimToPlay, FName(TEXT("DefaultSlot")), 0.1f, 0.15f, 1.35f);
				}
			}
		}
	}
	else if (FiringAnimSequence)
	{
		if (CharOwner && CharOwner->GetMesh())
		{
			if (UAnimInstance* AnimInst = CharOwner->GetMesh()->GetAnimInstance())
			{
				AnimInst->PlaySlotAnimationAsDynamicMontage(FiringAnimSequence, FName(TEXT("DefaultSlot")), 0.1f, 0.15f, 1.35f);
			}
		}
	}

	// Feedback visual procedimental de cuchillo en 1ra persona para el jugador local
	if (bIsMelee && CharOwner && CharOwner->IsLocallyControlled())
	{
		if (FirstPersonStaticMesh && FirstPersonStaticMesh->IsVisible())
		{
			const FVector RestLoc = FirstPersonWeaponOffset;
			const FRotator RestRot = FirstPersonWeaponRotation;
			const FVector ThrustLoc = RestLoc + FVector(12.0f, -4.0f, 2.0f);
			const FRotator SlashRot = RestRot + FRotator(8.0f, 12.0f, -18.0f);

			FirstPersonStaticMesh->SetRelativeLocationAndRotation(ThrustLoc, SlashRot);

			FTimerHandle RetractHandle;
			GetWorld()->GetTimerManager().SetTimer(RetractHandle, [this, RestLoc, RestRot]()
			{
				if (FirstPersonStaticMesh)
				{
					FirstPersonStaticMesh->SetRelativeLocationAndRotation(RestLoc, RestRot);
				}
			}, 0.12f, false);
		}
	}

	if (MuzzleFlashVFX)
	{
		USceneComponent* AttachComp = nullptr;
		if (FirstPersonMesh && FirstPersonMesh->IsVisible())
		{
			AttachComp = FirstPersonMesh;
		}
		else if (ThirdPersonMesh && ThirdPersonMesh->IsVisible())
		{
			AttachComp = ThirdPersonMesh;
		}
		else if (FirstPersonStaticMesh && FirstPersonStaticMesh->IsVisible())
		{
			AttachComp = FirstPersonStaticMesh;
		}
		else if (ThirdPersonStaticMesh && ThirdPersonStaticMesh->IsVisible())
		{
			AttachComp = ThirdPersonStaticMesh;
		}

		if (AttachComp)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				MuzzleFlashVFX,
				AttachComp,
				MuzzleSocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}
	}

	if (ImpactVFX && bHit && !HitLocation.IsNearlyZero())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactVFX, HitLocation, FRotator::ZeroRotator);
	}

	// Proyectil cosmético visible para clientes remotos (evita depender de la replicación del proyectil
	// del servidor que al viajar a 6000 u/s impacta en milisegundos y puede ocultarse antes de renderizar).
	if (!HasAuthority() && !bIsMelee && !HitLocation.IsNearlyZero())
	{
		const FVector TracerStart = GetMuzzleLocation();
		const FVector TracerDir = (HitLocation - TracerStart).GetSafeNormal();
		if (!TracerDir.IsNearlyZero())
		{
			FActorSpawnParameters TracerParams;
			TracerParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			UClass* ProjClass = ProjectileClass ? ProjectileClass.Get() : APEV2_ProjectileBase::StaticClass();
			if (APEV2_ProjectileBase* CosmeticProj = GetWorld()->SpawnActor<APEV2_ProjectileBase>(ProjClass, TracerStart, TracerDir.Rotation(), TracerParams))
			{
				CosmeticProj->SetReplicates(false);
				CosmeticProj->SetActorEnableCollision(false);
				CosmeticProj->InitializeBallistics(0.0f, 1.0f, ProjectileLaunchSpeed, ProjectileWeight, 0.0f, false, nullptr, nullptr);
				CosmeticProj->FireInDirection(TracerDir, ProjectileLaunchSpeed);
				CosmeticProj->SetLifeSpan(1.5f);
			}
		}
	}
}

void APEV2_WeaponBase::OnRep_CurrentAmmo()
{
	OnAmmoChangedEvent.Broadcast(CurrentAmmo, MaxMagazineAmmo);
}

void APEV2_WeaponBase::Multicast_PlayReloadFX_Implementation()
{
	bIsReloading = true;

	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
	if (CharOwner)
	{
		if (ReloadMontage)
		{
			CharOwner->PlayAnimMontage(ReloadMontage);
		}
		else if (ReloadAnimSequence)
		{
			if (USkeletalMeshComponent* Mesh = CharOwner->GetMesh())
			{
				if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
				{
					AnimInst->PlaySlotAnimationAsDynamicMontage(ReloadAnimSequence, FName(TEXT("DefaultSlot")), 0.1f, 0.15f, 1.0f);
				}
			}
		}
	}
}

void APEV2_WeaponBase::OnRep_IsReloading()
{
	// Sincronización del estado de recarga en clientes remotos
}
