// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Components/PEV2_CombatComponent.h"
#include "Characters/PEV2_Character.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

UPEV2_CombatComponent::UPEV2_CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	EquippedWeapon = nullptr;
	WeaponAttachSocket = FName(TEXT("HandGrip_R"));
}

void UPEV2_CombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPEV2_CombatComponent, EquippedWeapon);
}

void UPEV2_CombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// En el servidor, spawnear arma inicial si está configurada
	if (GetOwner()->HasAuthority() && DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APEV2_WeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<APEV2_WeaponBase>(DefaultWeaponClass, GetOwner()->GetActorTransform(), SpawnParams);
		if (SpawnedWeapon)
		{
			EquipWeapon(SpawnedWeapon);
		}
	}
}

void UPEV2_CombatComponent::StartFiring()
{
	APEV2_Character* CharOwner = Cast<APEV2_Character>(GetOwner());
	if (CharOwner && !CharOwner->CanPerformCombatActions())
	{
		return;
	}

	FVector AimStart = FVector::ZeroVector;
	FVector AimDir = FVector::ForwardVector;

	if (CharOwner)
	{
		FRotator ViewRot;
		CharOwner->GetActorEyesViewPoint(AimStart, ViewRot);
		AimDir = ViewRot.Vector();

		if (CharOwner->IsLocallyControlled() && CharOwner->GetFirstPersonCameraComponent())
		{
			AimStart = CharOwner->GetFirstPersonCameraComponent()->GetComponentLocation();
			AimDir = CharOwner->GetFirstPersonCameraComponent()->GetForwardVector();
		}
	}

	if (GetOwner()->HasAuthority())
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->StartFireWithAim(AimStart, AimDir);
		}
	}
	else
	{
		// En el cliente: llamar al Server RPC de disparo garantizado a través del CombatComponent
		Server_StartFiring(AimStart, AimDir);

		// Ejecución local inmediata si el arma está instanciada
		if (EquippedWeapon)
		{
			EquippedWeapon->StartFireWithAim(AimStart, AimDir);
		}
	}
}

void UPEV2_CombatComponent::StopFiring()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}

	if (!GetOwner()->HasAuthority())
	{
		Server_StopFiring();
	}
}

bool UPEV2_CombatComponent::Server_StartFiring_Validate(const FVector_NetQuantize& AimStart, const FVector_NetQuantizeNormal& AimDir)
{
	return true;
}

void UPEV2_CombatComponent::Server_StartFiring_Implementation(const FVector_NetQuantize& AimStart, const FVector_NetQuantizeNormal& AimDir)
{
	if (APEV2_Character* CharOwner = Cast<APEV2_Character>(GetOwner()))
	{
		if (!CharOwner->CanPerformCombatActions())
		{
			return;
		}
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->StartFireWithAim(AimStart, AimDir);
	}
}

bool UPEV2_CombatComponent::Server_StopFiring_Validate()
{
	return true;
}

void UPEV2_CombatComponent::Server_StopFiring_Implementation()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

void UPEV2_CombatComponent::Reload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Reload();
	}
}

void UPEV2_CombatComponent::EquipWeapon(APEV2_WeaponBase* NewWeapon)
{
	if (!GetOwner()->HasAuthority() || !NewWeapon)
	{
		return;
	}

	// Si ya había un arma equipada, destruirla o desvincularla
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = NewWeapon;
	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (CharacterOwner)
	{
		EquippedWeapon->AttachToOwnerCharacter(CharacterOwner, WeaponAttachSocket);
	}

	OnRep_EquippedWeapon();
}

void UPEV2_CombatComponent::OnRep_EquippedWeapon()
{
	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (CharacterOwner && EquippedWeapon)
	{
		EquippedWeapon->AttachToOwnerCharacter(CharacterOwner, WeaponAttachSocket);
	}

	OnEquippedWeaponChangedEvent.Broadcast(EquippedWeapon);
}
