// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Components/PEV2_CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
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
	if (EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
}

void UPEV2_CombatComponent::StopFiring()
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
