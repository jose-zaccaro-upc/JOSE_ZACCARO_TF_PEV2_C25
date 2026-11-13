// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "PEV2_Weapon_EpicTemplate.generated.h"

/**
 * Clase base C++ optimizada para las mallas esqueléticas (SKM) provenientes del template First Person de Epic Games.
 * Configura de fábrica la alineación 1:1, los sockets nativos ('Muzzle', 'Barrel') y el posicionamiento limpio
 * en cámara de primera persona sin artefactos de deformación.
 */
UCLASS(Blueprintable)
class TFPEV2_API APEV2_Weapon_EpicTemplate : public APEV2_WeaponBase
{
	GENERATED_BODY()

public:
	APEV2_Weapon_EpicTemplate();
};

/**
 * Pistola del template de Epic (SKM_Pistol).
 * Semiautomática, cargador de 15 balas, configurada con animaciones ABP_TP_Pistol.
 */
UCLASS()
class TFPEV2_API APEV2_Weapon_TemplatePistol : public APEV2_Weapon_EpicTemplate
{
	GENERATED_BODY()

public:
	APEV2_Weapon_TemplatePistol();
};

/**
 * Fusil de asalto del template de Epic (SKM_Rifle).
 * Automático, cargador de 30 balas, configurada con animaciones ABP_TP_Rifle.
 */
UCLASS()
class TFPEV2_API APEV2_Weapon_TemplateRifle : public APEV2_Weapon_EpicTemplate
{
	GENERATED_BODY()

public:
	APEV2_Weapon_TemplateRifle();
};

/**
 * Lanzagranadas del template de Epic (SKM_GrenadeLauncher).
 * Proyectil balístico explosivo, cargador de 6 granadas.
 */
UCLASS()
class TFPEV2_API APEV2_Weapon_TemplateLauncher : public APEV2_Weapon_EpicTemplate
{
	GENERATED_BODY()

public:
	APEV2_Weapon_TemplateLauncher();
};

