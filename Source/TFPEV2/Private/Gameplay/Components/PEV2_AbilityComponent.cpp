// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Components/PEV2_AbilityComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Gameplay/Weapons/PEV2_ProjectileBase.h"

UPEV2_AbilityComponent::UPEV2_AbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	CurrentAbilityData = nullptr;
	RemainingCharges = 0;
	CooldownEndTime = 0.0f;
	OriginalWalkSpeed = 600.0f;
}

void UPEV2_AbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPEV2_AbilityComponent, CurrentAbilityData);
	DOREPLIFETIME(UPEV2_AbilityComponent, RemainingCharges);
	DOREPLIFETIME(UPEV2_AbilityComponent, CooldownEndTime);
}

void UPEV2_AbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		{
			OriginalWalkSpeed = MoveComp->MaxWalkSpeed;
		}
	}
}

void UPEV2_AbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentAbilityData && CooldownEndTime > 0.0f)
	{
		const float Remaining = GetRemainingCooldownTime();
		OnAbilityCooldownUpdatedEvent.Broadcast(Remaining, CurrentAbilityData->CooldownDuration);
	}
}

void UPEV2_AbilityComponent::UseAbility()
{
	if (!CanUseAbility())
	{
		return;
	}

	Server_UseAbility();
}

bool UPEV2_AbilityComponent::Server_UseAbility_Validate()
{
	return true;
}

void UPEV2_AbilityComponent::Server_UseAbility_Implementation()
{
	if (!CanUseAbility() || !CurrentAbilityData)
	{
		return;
	}

	// Ejecutar la lógica específica en el Servidor
	ExecuteAbilityLogic(CurrentAbilityData);

	// Disminuir cargas
	RemainingCharges--;
	OnRep_RemainingCharges();

	// Activar Cooldown
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	CooldownEndTime = CurrentTime + CurrentAbilityData->CooldownDuration;
	OnRep_CooldownEndTime();

	// Efectos audiovisuales replicados
	Multicast_PlayAbilityFX(CurrentAbilityData);

	// Si las cargas llegaron a 0, se remueve la habilidad tras cumplirse el cooldown
	if (RemainingCharges <= 0)
	{
		FTimerHandle ClearAbilityTimer;
		GetWorld()->GetTimerManager().SetTimer(ClearAbilityTimer, [this]()
		{
			if (RemainingCharges <= 0)
			{
				CurrentAbilityData = nullptr;
				OnRep_CurrentAbilityData();
			}
		}, CurrentAbilityData->CooldownDuration, false);
	}
}

void UPEV2_AbilityComponent::ExecuteAbilityLogic(UPEV2_AbilityData* AbilityData)
{
	if (!AbilityData)
	{
		return;
	}

	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (!CharacterOwner)
	{
		return;
	}

	switch (AbilityData->AbilityType)
	{
	case EPEV2_AbilityType::Dash:
	{
		FVector DashDirection = CharacterOwner->GetVelocity();
		DashDirection.Z = 0.0f;

		if (DashDirection.IsNearlyZero())
		{
			DashDirection = CharacterOwner->GetActorForwardVector();
		}
		else
		{
			DashDirection.Normalize();
		}

		if (AbilityData->bDashInAimDirection)
		{
			DashDirection = CharacterOwner->GetControlRotation().Vector();
		}

		FVector LaunchVelocity = DashDirection * AbilityData->DashStrength;
		LaunchVelocity.Z = FMath::Max(LaunchVelocity.Z, 250.0f); // Pequeño despegue para evitar fricción con el piso

		CharacterOwner->LaunchCharacter(LaunchVelocity, true, true);
		break;
	}

	case EPEV2_AbilityType::Grenade:
	{
		if (AbilityData->GrenadeProjectileClass)
		{
			FVector SpawnLocation = CharacterOwner->GetPawnViewLocation() + (CharacterOwner->GetControlRotation().Vector() * 80.0f);
			FRotator SpawnRotation = CharacterOwner->GetControlRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = CharacterOwner;
			SpawnParams.Instigator = CharacterOwner;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			APEV2_ProjectileBase* Grenade = GetWorld()->SpawnActor<APEV2_ProjectileBase>(
				AbilityData->GrenadeProjectileClass,
				SpawnLocation,
				SpawnRotation,
				SpawnParams
			);

			if (Grenade)
			{
				Grenade->FireInDirection(SpawnRotation.Vector(), AbilityData->GrenadeThrowImpulse);
			}
		}
		break;
	}

	case EPEV2_AbilityType::SpeedBoost:
	{
		if (UCharacterMovementComponent* MoveComp = CharacterOwner->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = OriginalWalkSpeed * AbilityData->SpeedMultiplier;

			GetWorld()->GetTimerManager().ClearTimer(SpeedBoostTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(SpeedBoostTimerHandle, this, &UPEV2_AbilityComponent::ResetSpeedBoost, AbilityData->SpeedBoostDuration, false);
		}
		break;
	}

	case EPEV2_AbilityType::Shield:
	{
		// Si el personaje cuenta con TakeDamage o HealthComponent, se añade absorción temporal
		break;
	}

	default:
		break;
	}
}

void UPEV2_AbilityComponent::ResetSpeedBoost()
{
	if (ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = CharacterOwner->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = OriginalWalkSpeed;
		}
	}
}

void UPEV2_AbilityComponent::Multicast_PlayAbilityFX_Implementation(UPEV2_AbilityData* AbilityData)
{
	if (!AbilityData)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	const FVector Location = OwnerActor->GetActorLocation();

	if (AbilityData->ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AbilityData->ActivationSound, Location);
	}

	if (AbilityData->ActivationVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AbilityData->ActivationVFX, Location, OwnerActor->GetActorRotation());
	}
}

void UPEV2_AbilityComponent::EquipAbility(UPEV2_AbilityData* NewAbility)
{
	if (!GetOwner()->HasAuthority() || !NewAbility)
	{
		return;
	}

	CurrentAbilityData = NewAbility;
	RemainingCharges = NewAbility->MaxCharges;
	CooldownEndTime = 0.0f;

	OnRep_CurrentAbilityData();
	OnRep_RemainingCharges();
	OnRep_CooldownEndTime();
}

bool UPEV2_AbilityComponent::CanPickupAbility() const
{
	// Regla confirmada: Si no tiene habilidad, o se agotaron todas las cargas y el cooldown ya terminó
	if (!CurrentAbilityData)
	{
		return true;
	}

	const bool bHasCharges = RemainingCharges > 0;
	const bool bIsOnCooldown = GetRemainingCooldownTime() > 0.0f;

	return (!bHasCharges && !bIsOnCooldown);
}

bool UPEV2_AbilityComponent::CanUseAbility() const
{
	if (!CurrentAbilityData)
	{
		return false;
	}

	const bool bHasCharges = RemainingCharges > 0;
	const bool bCooldownReady = GetRemainingCooldownTime() <= 0.0f;

	return bHasCharges && bCooldownReady;
}

float UPEV2_AbilityComponent::GetRemainingCooldownTime() const
{
	if (CooldownEndTime <= 0.0f || !GetWorld())
	{
		return 0.0f;
	}

	const float Remaining = CooldownEndTime - GetWorld()->GetTimeSeconds();
	return FMath::Max(Remaining, 0.0f);
}

void UPEV2_AbilityComponent::OnRep_CurrentAbilityData()
{
	OnAbilityChangedEvent.Broadcast(CurrentAbilityData, RemainingCharges);
}

void UPEV2_AbilityComponent::OnRep_RemainingCharges()
{
	const int32 Max = CurrentAbilityData ? CurrentAbilityData->MaxCharges : 1;
	OnAbilityChargesChangedEvent.Broadcast(RemainingCharges, Max);
}

void UPEV2_AbilityComponent::OnRep_CooldownEndTime()
{
	const float Duration = CurrentAbilityData ? CurrentAbilityData->CooldownDuration : 1.0f;
	OnAbilityCooldownUpdatedEvent.Broadcast(GetRemainingCooldownTime(), Duration);
}
