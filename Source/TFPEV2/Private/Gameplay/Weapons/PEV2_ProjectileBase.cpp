// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Weapons/PEV2_ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

APEV2_ProjectileBase::APEV2_ProjectileBase()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(8.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionComp->OnComponentHit.AddDynamic(this, &APEV2_ProjectileBase::OnHit);
	RootComponent = CollisionComp;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BulletMesh(TEXT("/Game/alstra_infinite/PolyPack-Starter/Shooter-Essentials/Meshes/Props/Bullets/SM_Bullet_556_V1.SM_Bullet_556_V1"));
	if (BulletMesh.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(BulletMesh.Object);
		ProjectileMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 2.5f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackBulletMesh(TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Projectiles/Meshes/SM_FoamBullet.SM_FoamBullet"));
		if (FallbackBulletMesh.Succeeded())
		{
			ProjectileMesh->SetStaticMesh(FallbackBulletMesh.Object);
			ProjectileMesh->SetRelativeScale3D(FVector(2.0f, 0.8f, 0.8f));
		}
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TracerMat(TEXT("/Game/PEV2/Gameplay/Weapons/GrenadeLauncher/Materials/M_ProjectileBullet.M_ProjectileBullet"));
	if (TracerMat.Succeeded())
	{
		ProjectileMesh->SetMaterial(0, TracerMat.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 6000.0f;
	ProjectileMovement->MaxSpeed = 6000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.15f;
	ProjectileMovement->Bounciness = 0.3f;

	BaseDamage = 25.0f;
	HeadshotMultiplier = 2.0f;
	ProjectileWeight = 0.15f;
	InitialLaunchSpeed = 6000.0f;
	PhysicsImpulseScale = 15.0f;
	DamageRadius = 350.0f;
	bIsExplosive = false;

	InitialLifeSpan = 10.0f;
	bHasExploded = false;
	bExplodeOnImpact = false;
}

void APEV2_ProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APEV2_ProjectileBase, BaseDamage);
	DOREPLIFETIME(APEV2_ProjectileBase, HeadshotMultiplier);
	DOREPLIFETIME(APEV2_ProjectileBase, ProjectileWeight);
	DOREPLIFETIME(APEV2_ProjectileBase, InitialLaunchSpeed);
	DOREPLIFETIME(APEV2_ProjectileBase, bIsExplosive);
	DOREPLIFETIME(APEV2_ProjectileBase, bExplodeOnImpact);
	DOREPLIFETIME(APEV2_ProjectileBase, bIsAbilityProjectile);
}

void APEV2_ProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	ApplyBallisticSettings();

	if (HasAuthority() && bIsExplosive && FuseTime > 0.0f && !bExplodeOnImpact)
	{
		GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &APEV2_ProjectileBase::Explode, FuseTime, false);
	}
}

void APEV2_ProjectileBase::InitializeBallistics(
	float InDamage,
	float InHeadshotMultiplier,
	float InLaunchSpeed,
	float InWeight,
	float InImpulseScale,
	bool bInIsExplosive,
	UNiagaraSystem* InImpactVFX,
	USoundBase* InImpactSound,
	bool bInExplodeOnImpact)
{
	BaseDamage = InDamage;
	HeadshotMultiplier = InHeadshotMultiplier;
	InitialLaunchSpeed = InLaunchSpeed;
	ProjectileWeight = InWeight;
	PhysicsImpulseScale = InImpulseScale;
	bIsExplosive = bInIsExplosive;
	bExplodeOnImpact = bInExplodeOnImpact;

	if (InImpactVFX)
	{
		ExplosionVFX = InImpactVFX;
	}
	if (InImpactSound)
	{
		ExplosionSound = InImpactSound;
	}

	ApplyBallisticSettings();

	if (HasAuthority() && bIsExplosive && FuseTime > 0.0f && !bExplodeOnImpact)
	{
		GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &APEV2_ProjectileBase::Explode, FuseTime, false);
	}
}

void APEV2_ProjectileBase::ApplyBallisticSettings()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InitialLaunchSpeed;
		ProjectileMovement->MaxSpeed = InitialLaunchSpeed;
		ProjectileMovement->ProjectileGravityScale = ProjectileWeight;
		ProjectileMovement->bShouldBounce = bIsExplosive && !bExplodeOnImpact;
	}
}

void APEV2_ProjectileBase::OnRep_BallisticConfig()
{
	ApplyBallisticSettings();
}

void APEV2_ProjectileBase::FireInDirection(const FVector& ShootDirection, float InitialSpeed)
{
	InitialLaunchSpeed = InitialSpeed;
	ApplyBallisticSettings();

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = ShootDirection.GetSafeNormal() * InitialSpeed;
	}
}

void APEV2_ProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || bHasExploded)
	{
		return;
	}

	// Ignorar colisiones con el propio tirador o sus armas asociadas
	if (!OtherActor || OtherActor == GetInstigator() || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	if (GetInstigator() && (OtherActor->GetOwner() == GetInstigator() || OtherActor->GetAttachParentActor() == GetInstigator()))
	{
		return;
	}

	// 1. Transferencia de impulso cinético físico a objetos que simulan física y ragdolls
	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		const FVector ShotDir = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
		const float Speed = ProjectileMovement ? ProjectileMovement->Velocity.Size() : InitialLaunchSpeed;
		const float KineticImpulse = ProjectileWeight * Speed * PhysicsImpulseScale;
		OtherComp->AddImpulseAtLocation(ShotDir * KineticImpulse, Hit.ImpactPoint, Hit.BoneName);
	}

	// 2. Si el proyectil es explosivo (Lanzagranadas o Habilidad de Granada)
	if (bIsExplosive)
	{
		if (bExplodeOnImpact || Cast<APawn>(OtherActor))
		{
			Explode();
		}
		return;
	}

	// 3. Proyectil balístico directo (Pistola, Fusil, Subfusil, etc.)
	bHasExploded = true;

	AActor* TargetActor = OtherActor;
	if (TargetActor->GetOwner() && TargetActor->GetOwner()->IsA<ACharacter>())
	{
		TargetActor = TargetActor->GetOwner();
	}
	else if (TargetActor->GetAttachParentActor() && TargetActor->GetAttachParentActor()->IsA<ACharacter>())
	{
		TargetActor = TargetActor->GetAttachParentActor();
	}

	float FinalDamage = BaseDamage;
	bool bIsHeadshot = Hit.BoneName.ToString().ToLower().Contains(TEXT("head"));

	// Traza de seguridad contra la malla esquelética del personaje si impactó la cápsula externa
	if (!bIsHeadshot)
	{
		if (ACharacter* HitChar = Cast<ACharacter>(TargetActor))
		{
			if (USkeletalMeshComponent* MeshComp = HitChar->GetMesh())
			{
				FHitResult MeshHit;
				FCollisionQueryParams MeshParams;
				MeshParams.AddIgnoredActor(this);
				MeshParams.bTraceComplex = true;

				const FVector VelocityDir = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
				const FVector TraceStart = Hit.ImpactPoint - (VelocityDir * 15.0f);
				const FVector TraceEnd = Hit.ImpactPoint + (VelocityDir * 50.0f);

				if (MeshComp->LineTraceComponent(MeshHit, TraceStart, TraceEnd, MeshParams))
				{
					if (MeshHit.BoneName.ToString().ToLower().Contains(TEXT("head")))
					{
						bIsHeadshot = true;
					}
				}
			}
		}
	}

	if (bIsHeadshot)
	{
		FinalDamage *= HeadshotMultiplier;
	}

	UGameplayStatics::ApplyPointDamage(
		TargetActor,
		FinalDamage,
		ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector(),
		Hit,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);

	// Deshabilitar colisión y detener movimiento tras el impacto
	SetActorEnableCollision(false);
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}

	Multicast_PlayExplosionFX(Hit.ImpactPoint);
	SetLifeSpan(0.08f);
}

void APEV2_ProjectileBase::Explode()
{
	if (!HasAuthority() || bHasExploded)
	{
		return;
	}

	bHasExploded = true;
	const FVector ExplosionLocation = GetActorLocation();

	if (bIsExplosive)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(
			this,
			BaseDamage,
			ExplosionLocation,
			DamageRadius,
			UDamageType::StaticClass(),
			IgnoredActors,
			this,
			GetInstigatorController(),
			true
		);
	}

	Multicast_PlayExplosionFX(ExplosionLocation);
	Destroy();
}

void APEV2_ProjectileBase::Multicast_PlayExplosionFX_Implementation(FVector_NetQuantize ExplosionLocation)
{
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, ExplosionLocation);
	}

	if (ExplosionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, ExplosionLocation, FRotator::ZeroRotator);
	}
}
