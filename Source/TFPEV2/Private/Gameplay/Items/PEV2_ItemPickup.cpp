// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Items/PEV2_ItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Gameplay/Items/PEV2_AbilityData.h"
#include "Gameplay/Components/PEV2_AbilityComponent.h"

APEV2_ItemPickup::APEV2_ItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->InitSphereRadius(75.0f);
	SphereCollision->SetCollisionProfileName(TEXT("Trigger"));
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &APEV2_ItemPickup::HandleBeginOverlap);
	RootComponent = SphereCollision;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(RootComponent);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, 90.0f, 0.0f);

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(RootComponent);
	GlowLight->SetIntensity(3000.0f);
	GlowLight->SetAttenuationRadius(250.0f);

	bAutoPickupOnOverlap = true;
	bRespawn = true;
	RespawnTime = 10.0f;
	BobbingSpeed = 2.5f;
	BobbingHeight = 12.0f;
	RunningTime = 0.0f;
}

void APEV2_ItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (AbilityData)
	{
		if (AbilityData->PickupMesh && PickupMesh)
		{
			PickupMesh->SetStaticMesh(AbilityData->PickupMesh);
		}

		if (AbilityData->PickupMaterialOverride && PickupMesh)
		{
			PickupMesh->SetMaterial(0, AbilityData->PickupMaterialOverride);
		}

		if (GlowLight)
		{
			GlowLight->SetLightColor(AbilityData->PickupGlowColor);
		}
	}
}

void APEV2_ItemPickup::BeginPlay()
{
	Super::BeginPlay();

	if (PickupMesh)
	{
		InitialMeshRelativeLocation = PickupMesh->GetRelativeLocation();
	}
}

void APEV2_ItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Efecto de oscilación suave vertical
	if (PickupMesh && BobbingHeight > 0.0f)
	{
		RunningTime += DeltaTime * BobbingSpeed;
		const float DeltaHeight = FMath::Sin(RunningTime) * BobbingHeight;
		PickupMesh->SetRelativeLocation(InitialMeshRelativeLocation + FVector(0.0f, 0.0f, DeltaHeight));
	}
}

void APEV2_ItemPickup::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bAutoPickupOnOverlap)
	{
		return;
	}

	if (ACharacter* Char = Cast<ACharacter>(OtherActor))
	{
		ProcessPickup(Char);
	}
}

bool APEV2_ItemPickup::CanInteract_Implementation(ACharacter* Interactor) const
{
	if (!Interactor || !AbilityData)
	{
		return false;
	}

	if (UPEV2_AbilityComponent* AbilityComp = Interactor->FindComponentByClass<UPEV2_AbilityComponent>())
	{
		return AbilityComp->CanPickupAbility();
	}

	return false;
}

void APEV2_ItemPickup::Interact_Implementation(ACharacter* Interactor)
{
	if (HasAuthority() && Interactor)
	{
		ProcessPickup(Interactor);
	}
}

FText APEV2_ItemPickup::GetInteractionPrompt_Implementation(ACharacter* Interactor) const
{
	if (AbilityData)
	{
		return FText::Format(NSLOCTEXT("PEV2", "PickupPrompt", "Presiona [E] para recoger {0}"), AbilityData->AbilityName);
	}
	return FText::FromString(TEXT("Recoger"));
}

void APEV2_ItemPickup::ProcessPickup(ACharacter* TargetCharacter)
{
	if (!HasAuthority() || !TargetCharacter || !AbilityData)
	{
		return;
	}

	UPEV2_AbilityComponent* AbilityComp = TargetCharacter->FindComponentByClass<UPEV2_AbilityComponent>();
	if (!AbilityComp)
	{
		return;
	}

	// Regla de diseño: si ya tiene una habilidad activa, bloquear la recogida
	if (!AbilityComp->CanPickupAbility())
	{
		return;
	}

	// Equipar la habilidad al jugador
	AbilityComp->EquipAbility(AbilityData);

	// Notificar a todos los clientes para ocultar el pickup y reproducir audio
	Multicast_OnPickedUp();

	if (bRespawn)
	{
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &APEV2_ItemPickup::Multicast_OnRespawned, RespawnTime, false);
	}
	else
	{
		Destroy();
	}
}

void APEV2_ItemPickup::Multicast_OnPickedUp_Implementation()
{
	if (PickupMesh)
	{
		PickupMesh->SetVisibility(false, true);
	}

	if (GlowLight)
	{
		GlowLight->SetVisibility(false);
	}

	if (SphereCollision)
	{
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (AbilityData && AbilityData->PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AbilityData->PickupSound, GetActorLocation());
	}
}

void APEV2_ItemPickup::Multicast_OnRespawned_Implementation()
{
	if (PickupMesh)
	{
		PickupMesh->SetVisibility(true, true);
	}

	if (GlowLight)
	{
		GlowLight->SetVisibility(true);
	}

	if (SphereCollision)
	{
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}
