// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Characters/PEV2_Character.h"
#include "Gameplay/Components/PEV2_HealthComponent.h"
#include "Gameplay/Components/PEV2_CombatComponent.h"
#include "Gameplay/Components/PEV2_AbilityComponent.h"
#include "Interfaces/PEV2_InteractableInterface.h"
#include "Framework/PEV2_GameMode.h"
#include "Framework/PEV2_PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "UObject/ConstructorHelpers.h"

APEV2_Character::APEV2_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UPEV2_HealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UPEV2_CombatComponent>(TEXT("CombatComponent"));
	AbilityComponent = CreateDefaultSubobject<UPEV2_AbilityComponent>(TEXT("AbilityComponent"));

	// Cargar Input Actions por defecto del template si no han sido asignados en Blueprint
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionObj(TEXT("/Game/Input/Actions/IA_Jump"));
	if (JumpActionObj.Succeeded())
	{
		JumpAction = JumpActionObj.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionObj(TEXT("/Game/Input/Actions/IA_Move"));
	if (MoveActionObj.Succeeded())
	{
		MoveAction = MoveActionObj.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionObj(TEXT("/Game/Input/Actions/IA_Look"));
	if (LookActionObj.Succeeded())
	{
		LookAction = LookActionObj.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookActionObj(TEXT("/Game/Input/Actions/IA_MouseLook"));
	if (MouseLookActionObj.Succeeded())
	{
		MouseLookAction = MouseLookActionObj.Object;
	}
}

void APEV2_Character::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && HealthComponent)
	{
		HealthComponent->OnDeathEvent.AddDynamic(this, &APEV2_Character::OnDeath);
	}
}

void APEV2_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!JumpAction)
	{
		JumpAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	}
	if (!MoveAction)
	{
		MoveAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	}
	if (!LookAction)
	{
		LookAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	}
	if (!MouseLookAction)
	{
		MouseLookAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	}

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APEV2_Character::DoFireStart);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &APEV2_Character::DoFireEnd);
		}

		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &APEV2_Character::DoReload);
		}

		if (UseAbilityAction)
		{
			EnhancedInputComponent->BindAction(UseAbilityAction, ETriggerEvent::Started, this, &APEV2_Character::DoUseAbility);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APEV2_Character::DoInteract);
		}
	}

	// Garantizar que el IMC se aplique cuando el Pawn configura su input localmente
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (APEV2_PlayerController* PEV2_PC = Cast<APEV2_PlayerController>(PC))
		{
			PEV2_PC->AddDefaultMappingContext();
		}
	}
}

void APEV2_Character::DoFireStart()
{
	if (CombatComponent)
	{
		CombatComponent->StartFiring();
	}
}

void APEV2_Character::DoFireEnd()
{
	if (CombatComponent)
	{
		CombatComponent->StopFiring();
	}
}

void APEV2_Character::DoReload()
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void APEV2_Character::DoUseAbility()
{
	if (AbilityComponent)
	{
		AbilityComponent->UseAbility();
	}
}

void APEV2_Character::DoInteract()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->Implements<UPEV2_InteractableInterface>())
		{
			if (IPEV2_InteractableInterface::Execute_CanInteract(Actor, this))
			{
				IPEV2_InteractableInterface::Execute_Interact(Actor, this);
				break;
			}
		}
	}
}

FVector APEV2_Character::GetCombatAimLocation() const
{
	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);
	return EyeLocation + (EyeRotation.Vector() * 10000.0f);
}

FName APEV2_Character::GetWeaponAttachSocket(bool bFirstPerson) const
{
	return FName(TEXT("HandGrip_R"));
}

void APEV2_Character::OnDeath(AController* Killer, AActor* DamageCauser, const UDamageType* DamageType)
{
	Multicast_PlayEliminationFX();

	// Notify GameMode
	if (APEV2_GameMode* GM = GetWorld()->GetAuthGameMode<APEV2_GameMode>())
	{
		GM->OnPlayerKilled(GetController(), Killer, DamageCauser);
	}

	// Disable input and collision
	if (GetController())
	{
		GetController()->UnPossess();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
}

void APEV2_Character::Multicast_PlayEliminationFX_Implementation()
{
	// Trigger Ragdoll
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	}
}
