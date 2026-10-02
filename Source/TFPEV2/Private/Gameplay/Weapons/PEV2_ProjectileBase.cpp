// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Gameplay/Weapons/PEV2_ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"

APEV2_ProjectileBase::APEV2_ProjectileBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(12.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &APEV2_ProjectileBase::OnHit);
	RootComponent = CollisionComp;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;

	InitialLifeSpan = 10.0f;
	bHasExploded = false;
}

void APEV2_ProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bIsExplosive && FuseTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &APEV2_ProjectileBase::Explode, FuseTime, false);
	}
}

void APEV2_ProjectileBase::FireInDirection(const FVector& ShootDirection, float InitialSpeed)
{
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

	// Si impacta directamente contra un personaje, explotar de inmediato
	if (OtherActor && OtherActor != GetInstigator() && OtherActor != this)
	{
		if (Cast<APawn>(OtherActor))
		{
			Explode();
		}
	}
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

void APEV2_ProjectileBase::Multicast_PlayExplosionFX_Implementation(FVector ExplosionLocation)
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
