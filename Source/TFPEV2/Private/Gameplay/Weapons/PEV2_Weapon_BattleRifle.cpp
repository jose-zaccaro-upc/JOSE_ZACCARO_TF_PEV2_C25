// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Weapons/PEV2_Weapon_BattleRifle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundBase.h"

APEV2_Weapon_BattleRifle::APEV2_Weapon_BattleRifle()
{
	WeaponName = FText::FromString(TEXT("Fusil de Asalto ZC-57"));
	DamagePerShot = 34.0f;
	HeadshotMultiplier = 2.0f;
	FireRate = 0.14f;
	MaxMagazineAmmo = 30;
	CurrentAmmo = 30;
	bIsAutomatic = true;

	ProjectileLaunchSpeed = 7500.0f;
	ProjectileWeight = 0.18f;
	ProjectileImpulseScale = 20.0f;
	SpreadAngle = 0.4f;

	MuzzleSocketName = FName(TEXT("Socket_Barrel"));
	bCompensateImportRotation = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMesh(TEXT("/Game/alstra_infinite/PolyPack-Starter/Shooter-Essentials/Meshes/Guns/ZC57/SK_ZC57_Rigged.SK_ZC57_Rigged"));
	if (RifleMesh.Succeeded())
	{
		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetSkeletalMeshAsset(RifleMesh.Object);
		}
		if (ThirdPersonMesh)
		{
			ThirdPersonMesh->SetSkeletalMeshAsset(RifleMesh.Object);
		}
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> FPAnimClass(TEXT("/Game/Variant_Shooter/Anims/ABP_FP_Weapon.ABP_FP_Weapon_C"));
	if (FPAnimClass.Succeeded())
	{
		FirstPersonAnimInstanceClass = FPAnimClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> TPAnimClass(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C"));
	if (TPAnimClass.Succeeded())
	{
		ThirdPersonAnimInstanceClass = TPAnimClass.Class;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FireSnd(TEXT("/Game/PEV2/Gameplay/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02"));
	if (FireSnd.Succeeded())
	{
		FireSound = FireSnd.Object;
	}
}

