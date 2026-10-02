// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/PEV2_BattleZone.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/PEV2_Character.h"
#include "Kismet/GameplayStatics.h"

APEV2_BattleZone::APEV2_BattleZone()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ZoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneMesh"));
	RootComponent = ZoneMesh;
	
	// Default to no collision so players can pass through
	ZoneMesh->SetCollisionProfileName(TEXT("NoCollision"));

	InitialRadius = 5000.0f;
	TargetRadius = 500.0f;
	CurrentRadius = InitialRadius;
	DamagePerTick = 10.0f;
	DamageTickRate = 1.0f;
	ShrinkDuration = 60.0f;
	
	bIsShrinking = false;
	ShrinkElapsedTime = 0.0f;
}

void APEV2_BattleZone::BeginPlay()
{
	Super::BeginPlay();

	UpdateZoneScale();

	if (HasAuthority())
	{
		// Start applying damage over time
		GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &APEV2_BattleZone::ApplyZoneDamage, DamageTickRate, true);
	}
}

void APEV2_BattleZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && bIsShrinking)
	{
		ShrinkElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(ShrinkElapsedTime / ShrinkDuration, 0.0f, 1.0f);
		
		CurrentRadius = FMath::Lerp(InitialRadius, TargetRadius, Alpha);
		UpdateZoneScale();

		if (Alpha >= 1.0f)
		{
			bIsShrinking = false; // Finished shrinking
		}
	}
}

void APEV2_BattleZone::StartShrinking()
{
	if (HasAuthority())
	{
		bIsShrinking = true;
		ShrinkElapsedTime = 0.0f;
		InitialRadius = CurrentRadius; // Shrink from where we currently are
	}
}

void APEV2_BattleZone::UpdateZoneScale()
{
	// Assuming the base cylinder mesh has a diameter of 100 units, a scale of 100 makes it 10,000 units.
	// Adjust the scaling factor based on your actual mesh bounds.
	// For a 100-unit cylinder, Scale = CurrentRadius / 50.0f
	float ScaleXY = CurrentRadius / 50.0f;
	SetActorScale3D(FVector(ScaleXY, ScaleXY, 100.0f)); // 100.0f Z scale makes it very tall
}

void APEV2_BattleZone::ApplyZoneDamage()
{
	if (!HasAuthority()) return;

	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APEV2_Character::StaticClass(), FoundCharacters);

	FVector ZoneCenter = GetActorLocation();
	ZoneCenter.Z = 0.0f; // Ignore Z for distance checks (cylinder)

	for (AActor* Actor : FoundCharacters)
	{
		if (APEV2_Character* Char = Cast<APEV2_Character>(Actor))
		{
			FVector CharLoc = Char->GetActorLocation();
			CharLoc.Z = 0.0f;

			float Distance = FVector::Dist(ZoneCenter, CharLoc);

			if (Distance > CurrentRadius)
			{
				UGameplayStatics::ApplyDamage(Char, DamagePerTick, nullptr, this, UDamageType::StaticClass());
			}
		}
	}
}

void APEV2_BattleZone::OnRep_CurrentRadius()
{
	UpdateZoneScale();
}

void APEV2_BattleZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APEV2_BattleZone, CurrentRadius);
}
