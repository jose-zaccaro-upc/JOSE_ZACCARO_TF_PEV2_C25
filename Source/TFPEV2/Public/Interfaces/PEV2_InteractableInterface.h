// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PEV2_InteractableInterface.generated.h"

class ACharacter;

UINTERFACE(MinimalAPI, Blueprintable)
class UPEV2_InteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interfaz para cualquier actor que pueda ser recogido o interactuado en el mundo (armas, ítems, habilidades).
 */
class TFPEV2_API IPEV2_InteractableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Verifica si el personaje actual puede interactuar o recoger este objeto.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PEV2|Interaction")
	bool CanInteract(ACharacter* Interactor) const;

	/**
	 * Ejecuta la interacción o recolección del objeto.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PEV2|Interaction")
	void Interact(ACharacter* Interactor);

	/**
	 * Retorna el texto para la interfaz o prompt en pantalla (ej. "Presiona [E] para recoger Dash").
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PEV2|Interaction")
	FText GetInteractionPrompt(ACharacter* Interactor) const;

	/**
	 * Determina si este objeto se recoge automáticamente al caminar por encima (Overlap) o requiere pulsar tecla.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PEV2|Interaction")
	bool IsAutoPickup() const;
};
