// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "PEV2_Weapon_Knife.generated.h"

/**
 * Cuchillo de combate como arma final de la Carrera de Armas (Gun Master Arena).
 * Utiliza la malla estática de alstra_infinite y requiere un golpe cuerpo a cuerpo letal para ganar.
 */
UCLASS()
class TFPEV2_API APEV2_Weapon_Knife : public APEV2_WeaponBase
{
	GENERATED_BODY()

public:
	APEV2_Weapon_Knife();
};

