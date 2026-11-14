// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Weapons/PEV2_Weapon_Knife.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "UObject/ConstructorHelpers.h"

APEV2_Weapon_Knife::APEV2_Weapon_Knife()
{
	WeaponName = FText::FromString(TEXT("Cuchillo de Combate"));
	DamagePerShot = 120.0f; // Letal en cuerpo a cuerpo
	HeadshotMultiplier = 1.0f;
	FireRate = 0.45f;
	MaxMagazineAmmo = 1;
	CurrentAmmo = 1;
	bIsAutomatic = false;
	bIsMelee = true;
	FireSound = nullptr;
	MeleeRange = 190.0f;
	MeleeRadius = 45.0f;
	ProjectileImpulseScale = 30.0f;

	// Desactivar mallas esqueléticas: el cuchillo sólo usa mallas estáticas (SM_CombatKnifeV1)
	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetSkeletalMeshAsset(nullptr);
		FirstPersonMesh->SetVisibility(false, true);
	}
	if (ThirdPersonMesh)
	{
		ThirdPersonMesh->SetSkeletalMeshAsset(nullptr);
		ThirdPersonMesh->SetVisibility(false, true);
	}

	// Orientación en socket de tercera persona (configurable desde Blueprint)
	ThirdPersonMeshRotation = FRotator::ZeroRotator;
	ThirdPersonMeshOffset = FVector::ZeroVector;
	ThirdPersonMeshScale = FVector(1.0f, 1.0f, 1.0f);

	// Vista en primera persona vinculada a la cámara
	FirstPersonWeaponOffset = FVector(28.0f, 13.0f, -14.0f);
	FirstPersonWeaponRotation = FRotator(0.0f, -90.0f, 90.0f);
	FirstPersonWeaponScale = FVector(1.0f, 1.0f, 1.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> KnifeMesh(TEXT("/Game/alstra_infinite/PolyPack-Starter/Shooter-Essentials/Meshes/Props/SM_CombatKnifeV1.SM_CombatKnifeV1"));
	if (KnifeMesh.Succeeded())
	{
		if (FirstPersonStaticMesh)
		{
			FirstPersonStaticMesh->SetStaticMesh(KnifeMesh.Object);
			FirstPersonStaticMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 90.0f));
			FirstPersonStaticMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		}
		if (ThirdPersonStaticMesh)
		{
			ThirdPersonStaticMesh->SetStaticMesh(KnifeMesh.Object);
			ThirdPersonStaticMesh->SetRelativeRotation(FRotator::ZeroRotator);
			ThirdPersonStaticMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		}
	}

	// Postura en 3ra persona: Unarmed (postura relajada/combate cuerpo a cuerpo, nunca pistola o rifle)
	static ConstructorHelpers::FClassFinder<UAnimInstance> TPAnimClass(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
	if (TPAnimClass.Succeeded())
	{
		ThirdPersonAnimInstanceClass = TPAnimClass.Class;
	}

	FirstPersonAnimInstanceClass = nullptr;

	// Secuencias de animación de ataque cuerpo a cuerpo (golpes y cortes)
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> Attack1(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01"));
	if (Attack1.Succeeded())
	{
		FiringAnimSequence = Attack1.Object;
		MeleeAttackSequences.Add(Attack1.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> Attack2(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02.MM_Attack_02"));
	if (Attack2.Succeeded())
	{
		MeleeAttackSequences.Add(Attack2.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> Attack3(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03.MM_Attack_03"));
	if (Attack3.Succeeded())
	{
		MeleeAttackSequences.Add(Attack3.Object);
	}
}
