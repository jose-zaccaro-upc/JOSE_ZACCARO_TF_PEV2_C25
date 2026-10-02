// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Components/PEV2_HealthComponent.h"
#include "Net/UnrealNetwork.h"

UPEV2_HealthComponent::UPEV2_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

void UPEV2_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only hook if we are server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UPEV2_HealthComponent::HandleTakeAnyDamage);
		}
	}
}

void UPEV2_HealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || IsDead())
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

	OnHealthChangedEvent.Broadcast(this, CurrentHealth, -Damage, DamageType);

	if (IsDead())
	{
		OnDeathEvent.Broadcast(InstigatedBy, DamageCauser, DamageType);
	}
}

void UPEV2_HealthComponent::OnRep_CurrentHealth()
{
	OnHealthChangedEvent.Broadcast(this, CurrentHealth, 0.0f, nullptr);
}

void UPEV2_HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPEV2_HealthComponent, CurrentHealth);
}
