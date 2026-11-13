// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Weapons/PEV2_Weapon_SMG.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundBase.h"

APEV2_Weapon_SMG::APEV2_Weapon_SMG()
{
	WeaponName = FText::FromString(TEXT("Subfusil MGP-7"));
	DamagePerShot = 18.0f;
	HeadshotMultiplier = 1.75f;
	FireRate = 0.09f; // Alta cadencia
	MaxMagazineAmmo = 35;
	CurrentAmmo = 35;
	bIsAutomatic = true;

	ProjectileLaunchSpeed = 6500.0f;
	ProjectileWeight = 0.12f;
	ProjectileImpulseScale = 12.0f;
	SpreadAngle = 1.2f;

	MuzzleSocketName = FName(TEXT("Socket_Barrel"));
	bCompensateImportRotation = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SMGMesh(TEXT("/Game/alstra_infinite/PolyPack-Starter/Shooter-Essentials/Meshes/Guns/MGP7/SK_MGP7_Rigged.SK_MGP7_Rigged"));
	if (SMGMesh.Succeeded())
	{
		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetSkeletalMeshAsset(SMGMesh.Object);
		}
		if (ThirdPersonMesh)
		{
			ThirdPersonMesh->SetSkeletalMeshAsset(SMGMesh.Object);
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
