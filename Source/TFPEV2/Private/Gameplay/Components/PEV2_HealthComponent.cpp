// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Components/PEV2_HealthComponent.h"
#include "Framework/PEV2_PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DamageType.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPEV2_HealthComponent::UPEV2_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	MaxShield = 50.0f;
	CurrentShield = 0.0f;
}

void UPEV2_HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPEV2_HealthComponent, CurrentHealth);
	DOREPLIFETIME(UPEV2_HealthComponent, CurrentShield);
}

void UPEV2_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Vincular delegados de daño únicamente en la autoridad
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UPEV2_HealthComponent::HandleTakeAnyDamage);
			MyOwner->OnTakePointDamage.AddDynamic(this, &UPEV2_HealthComponent::HandleTakePointDamage);
			MyOwner->OnTakeRadialDamage.AddDynamic(this, &UPEV2_HealthComponent::HandleTakeRadialDamage);
		}
	}
}

void UPEV2_HealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || IsDead())
	{
		return;
	}

	float DamageToHealth = Damage;

	// Mitigación por escudo primero
	if (CurrentShield > 0.0f)
	{
		const float ShieldAbsorbed = FMath::Min(CurrentShield, Damage);
		CurrentShield -= ShieldAbsorbed;
		DamageToHealth -= ShieldAbsorbed;
		OnShieldChangedEvent.Broadcast(this, CurrentShield, -ShieldAbsorbed, MaxShield);
	}

	// Daño restante a la salud
	if (DamageToHealth > 0.0f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - DamageToHealth, 0.0f, MaxHealth);
		OnHealthChangedEvent.Broadcast(this, CurrentHealth, -DamageToHealth, DamageType);
	}

	// Feedback háptico/UI al jugador herido (víctima)
	if (APawn* VictimPawn = Cast<APawn>(DamagedActor))
	{
		if (APEV2_PlayerController* VictimPC = Cast<APEV2_PlayerController>(VictimPawn->GetController()))
		{
			const FVector ImpactDir = DamageCauser ? (DamageCauser->GetActorLocation() - DamagedActor->GetActorLocation()).GetSafeNormal() : FVector::ForwardVector;
			VictimPC->Client_ReceiveHitFeedback(Damage, ImpactDir);
		}
	}

	// Feedback de impacto confirmado al atacante
	if (InstigatedBy && InstigatedBy != Cast<AController>(DamagedActor))
	{
		if (APEV2_PlayerController* AttackerPC = Cast<APEV2_PlayerController>(InstigatedBy))
		{
			AttackerPC->Client_NotifyHitConfirmed(false, IsDead());
		}
	}

	if (IsDead())
	{
		OnDeathEvent.Broadcast(InstigatedBy, DamageCauser, DamageType);
	}
}

void UPEV2_HealthComponent::HandleTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	// Si el disparo impactó en la cabeza, notificamos al atacante de un headshot
	const bool bIsHeadshot = BoneName.ToString().ToLower().Contains(TEXT("head"));
	if (InstigatedBy && bIsHeadshot)
	{
		if (APEV2_PlayerController* AttackerPC = Cast<APEV2_PlayerController>(InstigatedBy))
		{
			AttackerPC->Client_NotifyHitConfirmed(true, IsDead());
		}
	}
}

void UPEV2_HealthComponent::HandleTakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	// Las explosiones ya son procesadas por OnTakeAnyDamage
}

void UPEV2_HealthComponent::AddShield(float Amount, float Duration)
{
	if (!GetOwner()->HasAuthority() || Amount <= 0.0f)
	{
		return;
	}

	CurrentShield = FMath::Clamp(CurrentShield + Amount, 0.0f, MaxShield);
	OnShieldChangedEvent.Broadcast(this, CurrentShield, Amount, MaxShield);

	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ShieldExpireTimerHandle, [this]()
		{
			CurrentShield = 0.0f;
			OnShieldChangedEvent.Broadcast(this, CurrentShield, 0.0f, MaxShield);
		}, Duration, false);
	}
}

void UPEV2_HealthComponent::ResetHealthAndShield()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	CurrentHealth = MaxHealth;
	CurrentShield = 0.0f;
	OnHealthChangedEvent.Broadcast(this, CurrentHealth, 0.0f, nullptr);
	OnShieldChangedEvent.Broadcast(this, CurrentShield, 0.0f, MaxShield);
}

void UPEV2_HealthComponent::OnRep_CurrentHealth()
{
	OnHealthChangedEvent.Broadcast(this, CurrentHealth, 0.0f, nullptr);
}

void UPEV2_HealthComponent::OnRep_CurrentShield()
{
	OnShieldChangedEvent.Broadcast(this, CurrentShield, 0.0f, MaxShield);
}
