// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalGun.h"

#include "MultiplayerGameState.h"
#include "PortalProjectile.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
APortalGun::APortalGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bUseLineTrace = false;
	RefireRate = AltFireRate = 2.0f;
	// Portal gun doesnt care about magazine size
	MagazineSize = 0;
	bFullAuto = false;
	bCanAltFire = true;
	
}

// Called when the game starts or when spawned
void APortalGun::BeginPlay()
{
	Super::BeginPlay();
	
}

// PortalGun has no bullets, no need to use any repnotify 
void APortalGun::OnRep_CurrentBullets()
{
}

void APortalGun::Fire()
{
	if (!bIsFiring) return;
	
	if (HasAuthority())
	{
		ServerFire();
		return;
	}
	
}

void APortalGun::ServerFire()
{
	ExecuteFire();
}

void APortalGun::ExecuteFire()
{
	// Could use multicast for vfx, sounds etc 
	if (WeaponOwner)
	{
		WeaponOwner->PlayFiringMontage(FiringMontage);
		WeaponOwner->AddWeaponRecoil(FiringRecoil);
	}
	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);

	// Perform the actual hit detection (now on server)
	if (bUseLineTrace)
	{
		PerformLineTrace();
	}
	else if (ProjectileClass)
	{
		ShootPortalProjectile(true);
		FVector Target = WeaponOwner ? WeaponOwner->GetWeaponTargetLocation() : PawnOwner->GetActorForwardVector() * 10000.0f;
		FireProjectile(Target);
	}

	// Refire timer
	TimeOfLastShot = GetWorld()->GetTimeSeconds();
	if (bFullAuto)
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &APortalGun::Fire, RefireRate, false);
	else
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &APortalGun::FireCooldownExpired, RefireRate, false);
	
	// Whatever client sided fire callbacks may exist
	ClientOnFired();
	
}

void APortalGun::AltFire()
{
	if (!bIsAltFiring) return;
	
	if (HasAuthority())
	{
		ServerAltFire();
		return;
	}
}

void APortalGun::ExecuteAltFire()
{
	// Can wrap upper section to PerformFire func and seperate the fire and altfire timer resetting instead
	// Could use multicast for vfx, sounds etc like joao said
	if (WeaponOwner)
	{
		WeaponOwner->PlayFiringMontage(FiringMontage);
		WeaponOwner->AddWeaponRecoil(FiringRecoil);
	}
	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);

	// Perform the actual hit detection (now on server)
	if (bUseLineTrace)
	{
		PerformLineTrace();
	}
	else if (ProjectileClass)
	{
		ShootPortalProjectile(false);
		FVector Target = WeaponOwner ? WeaponOwner->GetWeaponTargetLocation() : PawnOwner->GetActorForwardVector() * 10000.0f;
		FireProjectile(Target);
	}

	// Refire timer
	TimeOfLastAltFire = GetWorld()->GetTimeSeconds();
	if (bFullAuto)
		GetWorld()->GetTimerManager().SetTimer(AltRefireTimer, this, &APortalGun::AltFire, AltFireRate, false);
	else
		GetWorld()->GetTimerManager().SetTimer(AltRefireTimer, this, &APortalGun::FireCooldownExpired, AltFireRate, false);
	
	// Whatever client sided fire callbacks may exist
	ClientOnFired();
}

void APortalGun::ShootPortalProjectile(bool isOrange)
{
	ProjectileToSpawnIsOrange = isOrange;
}

AShooterProjectile* APortalGun::FireProjectile(const FVector& TargetLocation)
{
	if (!HasAuthority()) return nullptr; // Only server should spawn projectile
	// get the projectile transform
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);
	
	// spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	auto* Projectile = GetWorld()->SpawnActor<APortalProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);
	
	if (Projectile)
	{
		Projectile->ChangeColour(ProjectileToSpawnIsOrange);
		Projectile->OnRep_ProjectileColour(); // Set server material colour
	}

	if (WeaponOwner)
	{
		// play the firing montage
		WeaponOwner->PlayFiringMontage(FiringMontage);

		// add recoil
		WeaponOwner->AddWeaponRecoil(FiringRecoil);
	}
	
	return Projectile;
	
}

// Called every frame
void APortalGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

