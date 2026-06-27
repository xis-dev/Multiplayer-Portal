// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"      // FPointDamageEvent thingy
#include "DrawDebugHelpers.h"          // Can be deleted afterwards
#include "MultiplayerPlayer.h"
#include "Portal.h"
#include "ShooterCharacter.h"
#include "GameFramework/PlayerController.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

// Shootable visibility trace channel
#define ShootableChannel ECC_GameTraceChannel2
AShooterWeapon::AShooterWeapon()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the first person mesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);

	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	FirstPersonMesh->bOnlyOwnerSee = true;

	// create the third person mesh
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);

	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	ThirdPersonMesh->bOwnerNoSee = true;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	// subscribe to the owner's destroyed delegate
	GetOwner()->OnDestroyed.AddDynamic(this, &AShooterWeapon::OnOwnerDestroyed);

	// cast the weapon owner
	WeaponOwner = Cast<IShooterWeaponHolder>(GetOwner());
	PawnOwner = Cast<APawn>(GetOwner());

	// fill the first ammo clip
	CurrentBullets = MagazineSize;

	// attach the meshes to the owner
	WeaponOwner->AttachWeaponMeshes(this);
}

void AShooterWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear both refire timers
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
	GetWorld()->GetTimerManager().ClearTimer(AltRefireTimer);
}

void AShooterWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	// ensure this weapon is destroyed when the owner is destroyed
	Destroy();
}

void AShooterWeapon::ActivateWeapon_Implementation()
{
	WeaponActive = true;
	// Set server's own player state
	OnRep_WeaponActive();
}



void AShooterWeapon::OnRep_WeaponActive()
{
	// Get owning holder for each client and notify it, no need to use multicast as player pawn is replicated, locally activate/deactivate the required pawn
	if (auto* owningPlayer = Cast<IShooterWeaponHolder>(GetOwner()))
	{
		if (WeaponActive)
		{
			owningPlayer->OnWeaponActivated(this);
		}
		else
		{
			owningPlayer->OnWeaponDeactivated(this);
		} 
			
	}
}

void AShooterWeapon::OnRep_CurrentBullets()
{
	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}
}

void AShooterWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterWeapon, WeaponActive);
	DOREPLIFETIME_CONDITION(AShooterWeapon, CurrentBullets, COND_OwnerOnly);
}

void AShooterWeapon::DeactivateWeapon_Implementation()
{
	WeaponActive = false;
	OnRep_WeaponActive(); // Server state
}

void AShooterWeapon::StartFiring()
{
	// raise the firing flag
	bIsFiring = true;

	// check how much time has passed since we last shot
	// this may be under the refire rate if the weapon shoots slow enough and the player is spamming the trigger
	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

	if (TimeSinceLastShot > RefireRate)
	{
		// fire the weapon right away
		Fire();

	} else {

		// if we're full auto, schedule the next shot
		if (bFullAuto)
		{
			GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, TimeSinceLastShot, false);
		}

	}
}

void AShooterWeapon::StopFiring()
{
	// lower the firing flag
	bIsFiring = false;

	// clear the refire timer
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::ServerAltFire_Implementation()
{
	ExecuteAltFire();
}

bool AShooterWeapon::ServerAltFire_Validate()
{
	// Alt fire validation, if one exists
	return bCanAltFire;
}

void AShooterWeapon::ExecuteAltFire()
{
	if (!bCanAltFire) return;
}

void AShooterWeapon::StartAltFiring()
{
	if (!bCanAltFire) return;
	// raise the alt firing flag
	bIsAltFiring = true;

	// check how much time has passed since we last shot
	// this may be under the refire rate if the weapon shoots slow enough and the player is spamming the trigger
	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastAltFire;

	if (TimeSinceLastShot > AltFireRate)
	{
		// fire the weapon right away
		AltFire();

	} else {

		// if we're full auto, schedule the next shot
		if (bFullAuto)
		{
			GetWorld()->GetTimerManager().SetTimer(AltRefireTimer, this, &AShooterWeapon::AltFire, TimeSinceLastShot, false);
		}

	}
}

void AShooterWeapon::StopAltFiring()
{
	if (!bCanAltFire) return;
	// lower the alt firing flag
	bIsAltFiring = false;

	// clear the alt refire timer
	GetWorld()->GetTimerManager().ClearTimer(AltRefireTimer);
}

void AShooterWeapon::AddBullets(int32 Amount)
{
	
	CurrentBullets = FMath::Clamp(CurrentBullets + Amount, 0, MagazineSize);
	if (WeaponOwner) // update HUD immediately
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	
}


void AShooterWeapon::Fire()
{
	if (!bIsFiring) return;
	
	if (GetLocalRole() == ROLE_Authority)
	{
		ServerFire();
		return;
	}
}

void AShooterWeapon::AltFire()
{
	if (!bCanAltFire) return;
	// Base alt fire implementation
}


void AShooterWeapon::ClientOnFired_Implementation()
{
	if (WeaponOwner)
	{
		WeaponOwner->AddWeaponRecoil(FiringRecoil);
	}
}

void AShooterWeapon::ExecuteFire()
{
	if (CurrentBullets <= 0) return;

	// Consume ammo
	CurrentBullets--;
	OnRep_CurrentBullets(); // Server state
	

	// Play effects (these will automatically replicate if they modify state, 
	// but for sounds/montages we'll call Multicast later – optional)
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
		FVector Target = WeaponOwner ? WeaponOwner->GetWeaponTargetLocation() : PawnOwner->GetActorForwardVector() * 10000.0f;
		FireProjectile(Target);
	}

	// Refire timer
	TimeOfLastShot = GetWorld()->GetTimeSeconds();
	if (bFullAuto)
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RefireRate, false);
	else
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);
	
	// Whatever client sided fire callbacks may exist
	ClientOnFired();
	
}

void AShooterWeapon::ServerFire_Implementation()
{
	if (CurrentBullets > 0)
		ExecuteFire();
}

bool AShooterWeapon::ServerFire_Validate()
{
	// We could prevent cheating by checking time since last, not needed though
	return true;
}
void AShooterWeapon::PerformLineTrace()
{
	if (!WeaponOwner || !PawnOwner) return;

	// Player controller for the pawn's owner
	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC) return;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector End = CamLoc + CamRot.Vector() * LineTraceRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(PawnOwner);
	Params.bTraceComplex = true;

	if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ShootableChannel, Params))
	{
		// We hit a portal, TODO: Change to using tags or channels
		if (Hit.GetActor()->ActorHasTag("PortalObject"))
		{
			// Successfully teleported line trace, return out, else continue as normal
			if (TeleportLineTraceThroughPortal(Hit, CamLoc, End, Params)) return;
		}
		float Damage = LineTraceDamage;
		bool bHeadshot = false;
		
		// Headshot detection using your HeadCollider tag
		if (Hit.Component.IsValid() && Hit.Component->ComponentHasTag(FName("PlayerHead")))
		{
			bHeadshot = true;
			Damage *= HeadshotMultiplier;
		}

		// Apply damage only if we're on the server
		if (GetLocalRole() == ROLE_Authority && Hit.GetActor())
		{
			FPointDamageEvent DamageEvent(Damage, Hit, CamRot.Vector(), nullptr);
			Hit.GetActor()->TakeDamage(Damage, DamageEvent, PawnOwner->GetController(), this);
		}
	}
}

bool AShooterWeapon::TeleportLineTraceThroughPortal(const FHitResult& Hit, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params)
{
	auto* PortalActor = Cast<APortal>(Hit.GetActor());
			
	if (!PortalActor) return false;
	if (auto* OtherPortal = PortalActor->GetOtherPortal())
	{
		// Transform hit location to hit portal local space then to its position relative to the target portal in world space
		FVector NewLoc = PortalActor->GetTransform().InverseTransformPosition(Hit.ImpactPoint);
		NewLoc = FQuat(FRotator(0.0f, 180.0f, 0.0f)).RotateVector(NewLoc); // Flip location
		NewLoc = OtherPortal->GetTransform().TransformPosition(NewLoc);
		FVector NewStart = NewLoc + (OtherPortal->GetActorForwardVector() * 1.0f); // Portal location and nudge it a bit forward so as to not start inside
				
		FVector TraceDir = (End - Start).GetSafeNormal(); 
		FVector NewDir = PortalActor->GetOutputDirectionRelativeToOtherPortal(TraceDir).GetSafeNormal();
    
		float DistToTravel = LineTraceRange - Hit.Distance;
		FVector NewEnd = NewStart + (NewDir * DistToTravel);
				
		FHitResult NewHit;
		// Start another trace from portal hit point to target
		if (GetWorld()->LineTraceSingleByChannel(NewHit, NewStart, NewEnd, ShootableChannel, Params))
		{
			DrawDebugLine(GetWorld(), NewStart, NewEnd, FColor::Green, false, 3.0f, 0, 5.0f);
			float Damage = LineTraceDamage;
			bool bHeadshot = false;

		
			// Headshot detection using HeadCollider tag
			if (NewHit.Component.IsValid() && NewHit.Component->ComponentHasTag(FName("PlayerHead")))
			{
				bHeadshot = true;
				Damage *= HeadshotMultiplier;
			}

			// Apply damage only if we're on the server
			if (GetLocalRole() == ROLE_Authority && NewHit.GetActor())
			{
				FPointDamageEvent DamageEvent(Damage, NewHit, NewDir, nullptr);
				NewHit.GetActor()->TakeDamage(Damage, DamageEvent, PawnOwner->GetController(), this);
			}
			return true;
		}
	}
	return false;
}

void AShooterWeapon::FireCooldownExpired()
{
	// notify the owner
	WeaponOwner->OnSemiWeaponRefire();
}



AShooterProjectile* AShooterWeapon::FireProjectile(const FVector& TargetLocation)
{
	if (!HasAuthority()) return nullptr;
	// get the projectile transform
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);
	
	// spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	AShooterProjectile* Projectile = GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);

	// play the firing montage
	WeaponOwner->PlayFiringMontage(FiringMontage);

	// add recoil
	WeaponOwner->AddWeaponRecoil(FiringRecoil);
	
	

	
	return Projectile;
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	// find the muzzle location
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

	// calculate the spawn location ahead of the muzzle
	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * 0.0f);

	// find the aim rotation vector while applying some variance to the target 
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));

	// return the built transform
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}
