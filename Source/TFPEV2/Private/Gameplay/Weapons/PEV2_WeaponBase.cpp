// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"

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

	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);
	ThirdPersonMesh->SetOwnerNoSee(true);
	ThirdPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentAmmo = 30;
	MaxMagazineAmmo = 30;
	DamagePerShot = 25.0f;
	FireRate = 0.15f;
	MaxRange = 10000.0f;
	bIsAutomatic = true;

	bIsFiring = false;
	bIsReloading = false;
}

void APEV2_WeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APEV2_WeaponBase, CurrentAmmo);
}

void APEV2_WeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentAmmo = MaxMagazineAmmo;
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

	// Vincular malla de tercera persona a la malla principal del cuerpo
	if (ThirdPersonMesh && InCharacter->GetMesh())
	{
		ThirdPersonMesh->AttachToComponent(InCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	}

	// Si el personaje cuenta con malla de primera persona (FirstPersonMesh)
	if (USkeletalMeshComponent* FPMesh = InCharacter->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (FirstPersonMesh)
		{
			FirstPersonMesh->AttachToComponent(FPMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
		}
	}
}

void APEV2_WeaponBase::StartFire()
{
	Server_StartFire();
}

void APEV2_WeaponBase::StopFire()
{
	Server_StopFire();
}

void APEV2_WeaponBase::Reload()
{
	Server_Reload();
}

bool APEV2_WeaponBase::Server_StartFire_Validate()
{
	return true;
}

void APEV2_WeaponBase::Server_StartFire_Implementation()
{
	if (bIsReloading || CurrentAmmo <= 0)
	{
		return;
	}

	bIsFiring = true;
	PerformServerShot();

	if (bIsAutomatic)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimerHandle, this, &APEV2_WeaponBase::PerformServerShot, FireRate, true);
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

bool APEV2_WeaponBase::Server_Reload_Validate()
{
	return true;
}

void APEV2_WeaponBase::Server_Reload_Implementation()
{
	if (bIsReloading || CurrentAmmo >= MaxMagazineAmmo)
	{
		return;
	}

	bIsReloading = true;
	Server_StopFire();

	// Tiempo de recarga simulado
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, [this]()
	{
		CurrentAmmo = MaxMagazineAmmo;
		bIsReloading = false;
		OnRep_CurrentAmmo();
	}, 1.8f, false);
}

void APEV2_WeaponBase::PerformServerShot()
{
	if (CurrentAmmo <= 0)
	{
		Server_StopFire();
		return;
	}

	CurrentAmmo--;
	OnRep_CurrentAmmo();

	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (!CharacterOwner)
	{
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	CharacterOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector TraceStart = EyeLocation;
	const FVector TraceEnd = TraceStart + (EyeRotation.Vector() * MaxRange);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(CharacterOwner);
	QueryParams.bTraceComplex = true;

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	const FVector EndLocation = bHit ? HitResult.ImpactPoint : TraceEnd;

	if (bHit && HitResult.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			DamagePerShot,
			EyeRotation.Vector(),
			HitResult,
			CharacterOwner->GetController(),
			this,
			UDamageType::StaticClass()
		);
	}

	Multicast_PlayFireFX(EndLocation);
}

void APEV2_WeaponBase::Multicast_PlayFireFX_Implementation(FVector HitLocation)
{
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (MuzzleFlashVFX && FirstPersonMesh)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlashVFX,
			FirstPersonMesh,
			MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	if (ImpactVFX && !HitLocation.IsNearlyZero())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactVFX, HitLocation, FRotator::ZeroRotator);
	}
}

void APEV2_WeaponBase::OnRep_CurrentAmmo()
{
	OnAmmoChangedEvent.Broadcast(CurrentAmmo, MaxMagazineAmmo);
}
