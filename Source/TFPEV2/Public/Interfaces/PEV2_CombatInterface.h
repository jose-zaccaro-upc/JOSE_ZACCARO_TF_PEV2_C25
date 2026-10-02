// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PEV2_CombatInterface.generated.h"

class UPEV2_CombatComponent;
class UPEV2_AbilityComponent;

UINTERFACE(MinimalAPI, Blueprintable)
class UPEV2_CombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interfaz para personajes o entidades que utilicen armas y el sistema dinámico de habilidades.
 */
class TFPEV2_API IPEV2_CombatInterface
{
	GENERATED_BODY()

public:
	/** Retorna el componente de combate responsable de armas e inventario */
	virtual UPEV2_CombatComponent* GetCombatComponent() const = 0;

	/** Retorna el componente de habilidades dinámicas */
	virtual UPEV2_AbilityComponent* GetAbilityComponent() const = 0;

	/** Retorna la ubicación calculada hacia donde apunta el jugador (Crosshair / Raycast) */
	virtual FVector GetCombatAimLocation() const = 0;

	/** Retorna el nombre del socket al cual vincular el arma */
	virtual FName GetWeaponAttachSocket(bool bFirstPerson) const = 0;
};
