#include "Characters/PEV2_Character.h"
#include "Gameplay/Components/PEV2_HealthComponent.h"
#include "Gameplay/Components/PEV2_CombatComponent.h"
#include "Gameplay/Components/PEV2_AbilityComponent.h"
#include "Gameplay/Items/PEV2_AbilityData.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "Gameplay/Weapons/PEV2_WeaponRoster.h"
#include "Interfaces/PEV2_InteractableInterface.h"
#include "Framework/PEV2_GameMode.h"
#include "Framework/PEV2_PlayerState.h"
#include "Framework/PEV2_PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

APEV2_Character::APEV2_Character()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	CurrentWeaponIndex = 0;
	MeshAppearanceIndex = -1;
	bUseMeshAppearanceVariations = false;
	ReplicatedAimPitch = 0.0f;
	bIsDeadCameraActive = false;
	DeathCameraDistance = 280.0f;
	DeathCameraHeightOffset = 130.0f;
	DeathCameraInterpSpeed = 4.5f;
	RagdollFocusBone = FName(TEXT("pelvis"));

	HealthComponent = CreateDefaultSubobject<UPEV2_HealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UPEV2_CombatComponent>(TEXT("CombatComponent"));
	AbilityComponent = CreateDefaultSubobject<UPEV2_AbilityComponent>(TEXT("AbilityComponent"));

	// Cargar roster predeterminado de Carrera de Armas con clases nativas y Blueprints válidos
	WeaponTierClasses.Add(APEV2_Weapon_TemplatePistol::StaticClass());
	WeaponTierClasses.Add(APEV2_Weapon_SMG::StaticClass());
	WeaponTierClasses.Add(APEV2_Weapon_BattleRifle::StaticClass());
	WeaponTierClasses.Add(APEV2_Weapon_TemplateLauncher::StaticClass());
	WeaponTierClasses.Add(APEV2_Weapon_Knife::StaticClass());

	// Garantizar que la cápsula bloquee el trazado de disparos (ECC_Visibility) e ignore la cámara
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	// Cámara en primera persona: fijar al centro de la cápsula a la altura de los ojos (Z=65) con leve avance frontal
	// para evitar distorsiones por AimOffset de huesos esqueléticos o desplazamientos laterales extraños.
	if (UCameraComponent* Cam = GetFirstPersonCameraComponent())
	{
		Cam->SetupAttachment(GetCapsuleComponent());
		Cam->SetRelativeLocation(FVector(10.0f, 0.0f, 65.0f));
		Cam->SetRelativeRotation(FRotator::ZeroRotator);
		Cam->bUsePawnControlRotation = true;
		Cam->bEnableFirstPersonFieldOfView = false;
		Cam->bEnableFirstPersonScale = false;
	}

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

	static ConstructorHelpers::FObjectFinder<UInputAction> FireActionObj(TEXT("/Game/Variant_Shooter/Input/Actions/IA_Shoot"));
	if (FireActionObj.Succeeded())
	{
		FireAction = FireActionObj.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ReloadActionObj(TEXT("/Game/Variant_Shooter/Input/Actions/IA_Reload"));
	if (ReloadActionObj.Succeeded())
	{
		ReloadAction = ReloadActionObj.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> AbilityActionObj(TEXT("/Game/Variant_Shooter/Input/Actions/IA_Ability"));
	if (AbilityActionObj.Succeeded())
	{
		UseAbilityAction = AbilityActionObj.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionObj(TEXT("/Game/Variant_Shooter/Input/Actions/IA_Interact"));
	if (InteractActionObj.Succeeded())
	{
		InteractAction = InteractActionObj.Object;
	}
}

void APEV2_Character::BeginPlay()
{
	Super::BeginPlay();

	// Garantizar en ejecución que la cápsula y la malla bloqueen el trazado de disparos (ECC_Visibility)
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	if (GetFirstPersonMesh())
	{
		// Ocultar la malla de cuerpo/brazos en primera persona para despejar totalmente la vista
		// La cámara se sitúa limpiamente por sobre el mango de las armas
		GetFirstPersonMesh()->SetVisibility(false, true);
	}

	if (UCameraComponent* Cam = GetFirstPersonCameraComponent())
	{
		Cam->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Cam->SetRelativeLocation(FVector(10.0f, 0.0f, 65.0f));
		Cam->SetRelativeRotation(FRotator::ZeroRotator);
		Cam->bUsePawnControlRotation = true;
		Cam->bEnableFirstPersonFieldOfView = false;
		Cam->bEnableFirstPersonScale = false;
	}

	if (HasAuthority())
	{
		if (HealthComponent)
		{
			HealthComponent->OnDeathEvent.AddDynamic(this, &APEV2_Character::OnDeath);
		}

		// Asignar apariencia aleatoria solo si está habilitado
		if (bUseMeshAppearanceVariations && MeshAppearanceIndex < 0)
		{
			MeshAppearanceIndex = FMath::RandRange(0, 3);
			ApplyCharacterAppearance();
		}

		EquipCurrentWeaponTier();

		if (AbilityComponent && !AbilityComponent->GetCurrentAbility())
		{
			if (DefaultAbilityData)
			{
				AbilityComponent->EquipAbility(DefaultAbilityData);
			}
			else if (UPEV2_AbilityData* FallbackAbility = LoadObject<UPEV2_AbilityData>(nullptr, TEXT("/Game/PEV2/Gameplay/Items/DA_Grenade.DA_Grenade")))
			{
				AbilityComponent->EquipAbility(FallbackAbility);
			}
		}
	}
	else
	{
		if (bUseMeshAppearanceVariations)
		{
			ApplyCharacterAppearance();
		}
	}
}

void APEV2_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Actualizar y sincronizar el pitch de apuntado del jugador local
	if (IsLocallyControlled())
	{
		const float CurrentPitch = FRotator::NormalizeAxis(GetControlRotation().Pitch);
		ReplicatedAimPitch = CurrentPitch;
		if (!HasAuthority())
		{
			Server_SetAimPitch(CurrentPitch);
		}
		else
		{
			RemoteViewPitch16 = FRotator::CompressAxisToShort(CurrentPitch);
		}
	}
	else if (HasAuthority())
	{
		RemoteViewPitch16 = FRotator::CompressAxisToShort(ReplicatedAimPitch);
	}

	UpdateAimPitchReplication();

	// Animación de cámara en tercera persona enfocando el cuerpo tras morir
	if (bIsDeadCameraActive && IsLocallyControlled())
	{
		if (UCameraComponent* Cam = GetFirstPersonCameraComponent())
		{
			FVector RagdollPos = GetActorLocation();
			if (GetMesh() && GetMesh()->DoesSocketExist(RagdollFocusBone))
			{
				RagdollPos = GetMesh()->GetSocketLocation(RagdollFocusBone);
			}

			// Orientar hacia el cuerpo
			const FRotator TargetRot = (RagdollPos - DeathCameraCurrentLoc).Rotation();
			DeathCameraCurrentRot = FMath::RInterpTo(DeathCameraCurrentRot, TargetRot, DeltaTime, DeathCameraInterpSpeed);

			// Calcular posición deseada alejada y por encima
			const FVector DesiredBack = -TargetRot.Vector() * DeathCameraDistance;
			const FVector DesiredTarget = RagdollPos + DesiredBack + FVector(0.0f, 0.0f, DeathCameraHeightOffset);

			// Traza de seguridad para que la cámara no atraviese paredes o suelo
			FHitResult CamHit;
			FCollisionQueryParams CamParams;
			CamParams.AddIgnoredActor(this);
			if (CombatComponent && CombatComponent->GetEquippedWeapon())
			{
				CamParams.AddIgnoredActor(CombatComponent->GetEquippedWeapon());
			}

			FVector FinalTarget = DesiredTarget;
			if (GetWorld()->SweepSingleByChannel(CamHit, RagdollPos + FVector(0.0f, 0.0f, 30.0f), DesiredTarget, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(14.0f), CamParams))
			{
				FinalTarget = CamHit.Location;
			}

			DeathCameraCurrentLoc = FMath::VInterpTo(DeathCameraCurrentLoc, FinalTarget, DeltaTime, DeathCameraInterpSpeed);

			Cam->SetWorldLocationAndRotation(DeathCameraCurrentLoc, DeathCameraCurrentRot);
		}
	}
}

void APEV2_Character::OnRep_AimPitch()
{
	UpdateAimPitchReplication();
}

void APEV2_Character::Server_SetAimPitch_Implementation(float NewPitch)
{
	ReplicatedAimPitch = NewPitch;
	RemoteViewPitch16 = FRotator::CompressAxisToShort(NewPitch);
	if (AController* C = GetController())
	{
		FRotator CR = C->GetControlRotation();
		CR.Pitch = NewPitch;
		C->SetControlRotation(CR);
	}
}

void APEV2_Character::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (HasAuthority())
	{
		RemoteViewPitch16 = FRotator::CompressAxisToShort(ReplicatedAimPitch);
	}
}

FRotator APEV2_Character::GetBaseAimRotation() const
{
	FRotator BaseRot = Super::GetBaseAimRotation();
	if (!IsLocallyControlled())
	{
		BaseRot.Pitch = ReplicatedAimPitch;
	}
	return BaseRot;
}

void APEV2_Character::UpdateAimPitchReplication()
{
	// En el jugador local usar el ControlRotation directamente; en remotos usar ReplicatedAimPitch
	const float AimPitch = IsLocallyControlled() ? FRotator::NormalizeAxis(GetControlRotation().Pitch) : ReplicatedAimPitch;

	auto UpdateAnimInstanceProps = [this, AimPitch](UAnimInstance* AnimInst)
	{
		if (!AnimInst) return;

		// 1. Recorrer dinámicamente todas las propiedades de rotador (FRotator) del AnimInstance
		// En ABP_TP_Pistol se llama "Offset", en ABP_TP_Rifle se llama "Aim offset" o "offset"
		for (TFieldIterator<FStructProperty> PropIt(AnimInst->GetClass()); PropIt; ++PropIt)
		{
			if (PropIt->Struct == TBaseStructure<FRotator>::Get())
			{
				const FString PName = PropIt->GetName().ToLower();
				if (PName == TEXT("aim offset") || PName == TEXT("aimoffset") || PName == TEXT("offset") || PName.StartsWith(TEXT("aim")))
				{
					FRotator* RotPtr = PropIt->ContainerPtrToValuePtr<FRotator>(AnimInst);
					if (RotPtr)
					{
						RotPtr->Pitch = AimPitch;
					}
				}
			}
		}

		// 2. Recorrer dinámicamente todas las propiedades numéricas de Pitch (float / double)
		for (TFieldIterator<FProperty> PropIt(AnimInst->GetClass()); PropIt; ++PropIt)
		{
			const FString PName = PropIt->GetName().ToLower();
			if (PName.Contains(TEXT("pitch")))
			{
				const bool bIsNormalized = PName.Contains(TEXT("pitchn"));
				const float Val = bIsNormalized ? FMath::Clamp(AimPitch / 90.0f, -1.0f, 1.0f) : AimPitch;

				if (FFloatProperty* FP = CastField<FFloatProperty>(*PropIt))
				{
					FP->SetPropertyValue_InContainer(AnimInst, Val);
				}
				else if (FDoubleProperty* DP = CastField<FDoubleProperty>(*PropIt))
				{
					DP->SetPropertyValue_InContainer(AnimInst, static_cast<double>(Val));
				}
			}
		}

		// 3. Sincronizar referencia de First Person Camera si el AnimInstance la requiere (previene Blueprint Accessed None)
		static const FName CamPropNames[] = {
			FName(TEXT("First Person Camera")),
			FName(TEXT("FirstPersonCamera")),
			FName(TEXT("FirstPersonCameraComponent")),
			FName(TEXT("CameraComponent"))
		};
		for (const FName& CName : CamPropNames)
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(AnimInst->GetClass()->FindPropertyByName(CName)))
			{
				ObjProp->SetPropertyValue_InContainer(AnimInst, GetFirstPersonCameraComponent());
				break;
			}
		}
	};

	if (GetMesh())
	{
		UpdateAnimInstanceProps(GetMesh()->GetAnimInstance());
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
	if (!FireAction)
	{
		FireAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Variant_Shooter/Input/Actions/IA_Shoot.IA_Shoot"));
	}
	if (!ReloadAction)
	{
		ReloadAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Variant_Shooter/Input/Actions/IA_Reload.IA_Reload"));
		if (!ReloadAction)
		{
			ReloadAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Reload.IA_Reload"));
		}
	}
	if (!UseAbilityAction)
	{
		UseAbilityAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Variant_Shooter/Input/Actions/IA_Ability.IA_Ability"));
		if (!UseAbilityAction)
		{
			UseAbilityAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Ability.IA_Ability"));
		}
	}
	if (!InteractAction)
	{
		InteractAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Variant_Shooter/Input/Actions/IA_Interact.IA_Interact"));
		if (!InteractAction)
		{
			InteractAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Interact.IA_Interact"));
		}
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

	// Fallback directo de teclas (Click Izquierdo = Disparar, R = Recargar, Q = Habilidad, E = Interactuar)
	// Asegura funcionamiento inmediato en PIE para clientes y servidor aún antes de configurar los assets en el IMC
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &APEV2_Character::DoFireStart);
		PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &APEV2_Character::DoFireEnd);
		PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &APEV2_Character::DoReload);
		PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &APEV2_Character::DoUseAbility);
		PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &APEV2_Character::DoInteract);
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

bool APEV2_Character::CanPerformCombatActions() const
{
	if (bIsDeadCameraActive)
	{
		return false;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			if (GS->HasMatchEnded())
			{
				return false;
			}
		}
	}

	return true;
}

void APEV2_Character::DoFireStart()
{
	if (!CanPerformCombatActions())
	{
		return;
	}

	const uint64 CurrentFrame = GFrameCounter;
	if (CurrentFrame == LastFireInputFrame)
	{
		return;
	}
	LastFireInputFrame = CurrentFrame;

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
	if (!CanPerformCombatActions())
	{
		return;
	}

	const uint64 CurrentFrame = GFrameCounter;
	if (CurrentFrame == LastReloadInputFrame)
	{
		return;
	}
	LastReloadInputFrame = CurrentFrame;

	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void APEV2_Character::DoUseAbility()
{
	if (!CanPerformCombatActions())
	{
		return;
	}

	const uint64 CurrentFrame = GFrameCounter;
	if (CurrentFrame == LastAbilityInputFrame)
	{
		return;
	}
	LastAbilityInputFrame = CurrentFrame;

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
	if (GetMesh())
	{
		if (GetMesh()->DoesSocketExist(FName(TEXT("HandGrip_R"))))
		{
			return FName(TEXT("HandGrip_R"));
		}
		if (GetMesh()->DoesSocketExist(FName(TEXT("hand_r"))))
		{
			return FName(TEXT("hand_r"));
		}
	}
	return FName(TEXT("HandGrip_R"));
}

void APEV2_Character::OnDeath(AController* Killer, AActor* DamageCauser, const UDamageType* DamageType)
{
	if (CombatComponent)
	{
		CombatComponent->StopFiring();
	}

	Multicast_PlayEliminationFX();

	// Notificar al PlayerController local para que inicie la UI "HAS MUERTO" con el tiempo de respawn
	if (APEV2_PlayerController* PC = Cast<APEV2_PlayerController>(GetController()))
	{
		const float RespawnTime = 3.0f;
		PC->Client_NotifyPlayerDied(RespawnTime);
	}

	// Notify GameMode
	if (APEV2_GameMode* GM = GetWorld()->GetAuthGameMode<APEV2_GameMode>())
	{
		GM->OnPlayerKilled(GetController(), Killer, DamageCauser);
	}

	// Deshabilitar movimiento y colisiones de la cápsula
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
}

void APEV2_Character::Multicast_PlayEliminationFX_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->StopFiring();
	}

	// Trigger Ragdoll
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	}

	// Si es el cliente dueño, cambiar la perspectiva para mostrar el cuerpo ragdoll
	if (IsLocallyControlled())
	{
		if (GetMesh())
		{
			GetMesh()->SetOwnerNoSee(false);
		}

		if (GetFirstPersonMesh())
		{
			GetFirstPersonMesh()->SetVisibility(false, true);
		}

		if (CombatComponent && CombatComponent->GetEquippedWeapon())
		{
			CombatComponent->GetEquippedWeapon()->SetThirdPersonDeathView(true);
		}

		if (UCameraComponent* Cam = GetFirstPersonCameraComponent())
		{
			Cam->bUsePawnControlRotation = false;
			DeathCameraCurrentLoc = Cam->GetComponentLocation();
			DeathCameraCurrentRot = Cam->GetComponentRotation();
			bIsDeadCameraActive = true;
		}
	}
}

void APEV2_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APEV2_Character, CurrentWeaponIndex);
	DOREPLIFETIME(APEV2_Character, MeshAppearanceIndex);
	DOREPLIFETIME(APEV2_Character, ReplicatedAimPitch);
}

void APEV2_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (CombatComponent && CombatComponent->GetEquippedWeapon())
		{
			CombatComponent->GetEquippedWeapon()->SetOwner(this);
		}

		if (APEV2_PlayerState* PS = GetPlayerState<APEV2_PlayerState>())
		{
			RestoreFromPlayerState(PS);
		}
	}
}

void APEV2_Character::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (CombatComponent && CombatComponent->GetEquippedWeapon())
	{
		CombatComponent->GetEquippedWeapon()->SetOwner(this);
		CombatComponent->GetEquippedWeapon()->AttachToOwnerCharacter(this, FName(TEXT("HandGrip_R")));
	}
}

void APEV2_Character::AdvanceWeaponTier()
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentWeaponIndex++;
	if (APEV2_PlayerState* PS = GetPlayerState<APEV2_PlayerState>())
	{
		PS->SavedWeaponTier = CurrentWeaponIndex;
	}
	EquipCurrentWeaponTier();
	OnRep_CurrentWeaponIndex();
}

void APEV2_Character::EquipCurrentWeaponTier()
{
	if (!HasAuthority() || !CombatComponent || WeaponTierClasses.Num() == 0)
	{
		return;
	}

	const int32 SafeIndex = FMath::Clamp(CurrentWeaponIndex, 0, WeaponTierClasses.Num() - 1);
	if (WeaponTierClasses.IsValidIndex(SafeIndex) && WeaponTierClasses[SafeIndex])
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (APEV2_WeaponBase* NewWeapon = GetWorld()->SpawnActor<APEV2_WeaponBase>(WeaponTierClasses[SafeIndex], GetActorTransform(), SpawnParams))
		{
			CombatComponent->EquipWeapon(NewWeapon);
		}
	}
}

void APEV2_Character::ResetForRespawn()
{
	if (!HasAuthority())
	{
		return;
	}

	if (HealthComponent)
	{
		HealthComponent->ResetHealthAndShield();
	}

	Multicast_ResetRespawnFX();
	EquipCurrentWeaponTier();
}

void APEV2_Character::OnRep_CurrentWeaponIndex()
{
	OnWeaponTierChangedEvent.Broadcast(CurrentWeaponIndex, WeaponTierClasses.Num());
}

void APEV2_Character::OnRep_MeshAppearanceIndex()
{
	if (bUseMeshAppearanceVariations)
	{
		ApplyCharacterAppearance();
	}
}

void APEV2_Character::ApplyCharacterAppearance()
{
	if (!bUseMeshAppearanceVariations || !GetMesh() || MeshAppearanceIndex < 0)
	{
		return;
	}

	static const TCHAR* MannyPath = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple");
	static const TCHAR* QuinnPath = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple");

	static const TCHAR* MannyMat01 = TEXT("/Game/Characters/Mannequins/Materials/Manny/MI_Manny_01_New.MI_Manny_01_New");
	static const TCHAR* MannyMat02 = TEXT("/Game/Characters/Mannequins/Materials/Manny/MI_Manny_02_New.MI_Manny_02_New");
	static const TCHAR* QuinnMat01 = TEXT("/Game/Characters/Mannequins/Materials/Quinn/MI_Quinn_01.MI_Quinn_01");
	static const TCHAR* QuinnMat02 = TEXT("/Game/Characters/Mannequins/Materials/Quinn/MI_Quinn_02.MI_Quinn_02");

	// Variación 0: Manny Camo A (MI_Manny_01_New)
	// Variación 1: Manny Camo B (MI_Manny_02_New)
	// Variación 2: Quinn SpecOps A (MI_Quinn_01)
	// Variación 3: Quinn SpecOps B (MI_Quinn_02)
	const int32 SafeIdx = FMath::Abs(MeshAppearanceIndex) % 4;

	USkeletalMesh* TargetMesh = nullptr;
	UMaterialInterface* TargetMat0 = nullptr;
	UMaterialInterface* TargetMat1 = nullptr;

	if (SafeIdx <= 1)
	{
		TargetMesh = LoadObject<USkeletalMesh>(nullptr, MannyPath);
		const TCHAR* SelectedMat = (SafeIdx == 0) ? MannyMat01 : MannyMat02;
		TargetMat0 = LoadObject<UMaterialInterface>(nullptr, SelectedMat);
		TargetMat1 = LoadObject<UMaterialInterface>(nullptr, SelectedMat);
	}
	else
	{
		TargetMesh = LoadObject<USkeletalMesh>(nullptr, QuinnPath);
		const TCHAR* SelectedMat = (SafeIdx == 2) ? QuinnMat01 : QuinnMat02;
		TargetMat0 = LoadObject<UMaterialInterface>(nullptr, SelectedMat);
		TargetMat1 = LoadObject<UMaterialInterface>(nullptr, SelectedMat);
	}

	if (TargetMesh && GetMesh()->GetSkeletalMeshAsset() != TargetMesh)
	{
		GetMesh()->SetSkeletalMeshAsset(TargetMesh);

		// Al cambiar el SkeletalMeshAsset, UE reinicia el AnimInstance al valor por defecto (ABP_Unarmed).
		// Si el jugador ya tiene un arma equipada, reasignamos el ThirdPersonAnimInstanceClass del arma.
		if (CombatComponent && CombatComponent->GetEquippedWeapon())
		{
			if (TSubclassOf<UAnimInstance> TPAnimClass = CombatComponent->GetEquippedWeapon()->GetThirdPersonAnimInstanceClass())
			{
				GetMesh()->SetAnimInstanceClass(TPAnimClass);
			}
		}
		else
		{
			if (UClass* UnarmedAnim = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C")))
			{
				GetMesh()->SetAnimInstanceClass(UnarmedAnim);
			}
		}
	}

	if (TargetMat0)
	{
		GetMesh()->SetMaterial(0, TargetMat0);
	}
	if (TargetMat1)
	{
		GetMesh()->SetMaterial(1, TargetMat1);
	}

	if (GetFirstPersonMesh())
	{
		GetFirstPersonMesh()->SetVisibility(false, true);
	}
}

void APEV2_Character::RestoreFromPlayerState(APEV2_PlayerState* InPS)
{
	if (!InPS || !HasAuthority())
	{
		return;
	}

	CurrentWeaponIndex = InPS->SavedWeaponTier;
	MeshAppearanceIndex = InPS->CharacterAppearanceIndex;

	if (bUseMeshAppearanceVariations)
	{
		ApplyCharacterAppearance();
	}
	EquipCurrentWeaponTier();
	OnRep_CurrentWeaponIndex();
}

void APEV2_Character::Multicast_ResetRespawnFX_Implementation()
{
	bIsDeadCameraActive = false;

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(false);
		MeshComp->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		MeshComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
		MeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
		MeshComp->SetOwnerNoSee(true);
	}

	if (GetFirstPersonMesh())
	{
		GetFirstPersonMesh()->SetVisibility(false, true);
	}

	if (UCameraComponent* Cam = GetFirstPersonCameraComponent())
	{
		Cam->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Cam->SetRelativeLocation(FVector(10.0f, 0.0f, 65.0f));
		Cam->SetRelativeRotation(FRotator::ZeroRotator);
		Cam->bUsePawnControlRotation = true;
		Cam->bEnableFirstPersonFieldOfView = false;
		Cam->bEnableFirstPersonScale = false;
	}

	if (CombatComponent && CombatComponent->GetEquippedWeapon())
	{
		CombatComponent->GetEquippedWeapon()->SetThirdPersonDeathView(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}
