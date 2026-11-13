// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Weapons/PEV2_Weapon_EpicTemplate.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundBase.h"

// ========================================================
// APEV2_Weapon_EpicTemplate (Base)
// ========================================================
APEV2_Weapon_EpicTemplate::APEV2_Weapon_EpicTemplate()
{
	// Las mallas nativas de Epic (SKM_Pistol, SKM_Rifle, SKM_GrenadeLauncher) no requieren compensación de 90 grados
	bCompensateImportRotation = false;
	ThirdPersonMeshRotation = FRotator::ZeroRotator;

	// Alineación estándar para la cámara en 1ra persona
	FirstPersonWeaponOffset = FVector(28.0f, 13.0f, -15.0f);
	FirstPersonWeaponRotation = FRotator(-1.5f, -1.0f, 0.0f);

	// Socket predeterminado del cañón
	MuzzleSocketName = FName(TEXT("Muzzle"));
	WeaponAttachSocket = FName(TEXT("HandGrip_R"));

	// Cargar sonido genérico de disparo de Epic
	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultFireSound(TEXT("/Game/PEV2/Gameplay/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02"));
	if (DefaultFireSound.Succeeded())
	{
		FireSound = DefaultFireSound.Object;
	}
}

// ========================================================
// APEV2_Weapon_TemplatePistol (SKM_Pistol)
// ========================================================
APEV2_Weapon_TemplatePistol::APEV2_Weapon_TemplatePistol()
{
	WeaponName = FText::FromString(TEXT("Pistola Táctica Epic"));
	DamagePerShot = 26.0f;
	HeadshotMultiplier = 2.0f;
	FireRate = 0.18f;
	MaxMagazineAmmo = 15;
	CurrentAmmo = 15;
	bIsAutomatic = false;
	SpreadAngle = 0.6f;
	ProjectileLaunchSpeed = 6000.0f;
	ProjectileWeight = 0.15f;
	ProjectileImpulseScale = 15.0f;

	MuzzleSocketName = FName(TEXT("Barrel"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PistolMesh(TEXT("/Game/PEV2/Gameplay/Weapons/Pistol/Meshes/SKM_Pistol.SKM_Pistol"));
	if (PistolMesh.Succeeded())
	{
		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetSkeletalMeshAsset(PistolMesh.Object);
		}
		if (ThirdPersonMesh)
		{
			ThirdPersonMesh->SetSkeletalMeshAsset(PistolMesh.Object);
		}
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> TPAnimClass(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Pistol.ABP_TP_Pistol_C"));
	if (TPAnimClass.Succeeded())
	{
		ThirdPersonAnimInstanceClass = TPAnimClass.Class;
	}
}

// ========================================================
// APEV2_Weapon_TemplateRifle (SKM_Rifle)
// ========================================================
APEV2_Weapon_TemplateRifle::APEV2_Weapon_TemplateRifle()
{
	WeaponName = FText::FromString(TEXT("Fusil de Asalto Epic"));
	DamagePerShot = 32.0f;
	HeadshotMultiplier = 2.0f;
	FireRate = 0.12f;
	MaxMagazineAmmo = 30;
	CurrentAmmo = 30;
	bIsAutomatic = true;
	SpreadAngle = 0.4f;
	ProjectileLaunchSpeed = 7500.0f;
	ProjectileWeight = 0.18f;
	ProjectileImpulseScale = 20.0f;

	MuzzleSocketName = FName(TEXT("Muzzle"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMesh(TEXT("/Game/PEV2/Gameplay/Weapons/Rifle/Meshes/SKM_Rifle.SKM_Rifle"));
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

	static ConstructorHelpers::FClassFinder<UAnimInstance> TPAnimClass(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C"));
	if (TPAnimClass.Succeeded())
	{
		ThirdPersonAnimInstanceClass = TPAnimClass.Class;
	}
}

// ========================================================
// APEV2_Weapon_TemplateLauncher (SKM_GrenadeLauncher)
// ========================================================
APEV2_Weapon_TemplateLauncher::APEV2_Weapon_TemplateLauncher()
{
	WeaponName = FText::FromString(TEXT("Lanzagranadas Epic"));
	DamagePerShot = 110.0f;
	HeadshotMultiplier = 1.0f;
	FireRate = 0.85f;
	MaxMagazineAmmo = 6;
	CurrentAmmo = 6;
	bIsAutomatic = false;
	bSpawnExplosiveProjectile = true;
	SpreadAngle = 0.2f;
	ProjectileLaunchSpeed = 3000.0f;
	ProjectileWeight = 0.5f;
	ProjectileImpulseScale = 50.0f;

	MuzzleSocketName = FName(TEXT("Muzzle"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> LauncherMesh(TEXT("/Game/PEV2/Gameplay/Weapons/GrenadeLauncher/Meshes/SKM_GrenadeLauncher.SKM_GrenadeLauncher"));
	if (LauncherMesh.Succeeded())
	{
		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetSkeletalMeshAsset(LauncherMesh.Object);
		}
		if (ThirdPersonMesh)
		{
			ThirdPersonMesh->SetSkeletalMeshAsset(LauncherMesh.Object);
		}
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> TPAnimClass(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C"));
	if (TPAnimClass.Succeeded())
	{
		ThirdPersonAnimInstanceClass = TPAnimClass.Class;
	}
}

