// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "MultiplayerGameMode.h"
#include "MultiplayerPortalPlayerStats.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Variant_Shooter/Weapons/Rifle.h"
#include "Variant_Shooter/Weapons/Pistol.h"
#include "Variant_Shooter/Weapons/GrenadeLauncher.h"
#include "Net/UnrealNetwork.h"
#include "ShooterGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterUI.h"
#include "Components/BoxComponent.h"


AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));


	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();


	// reset HP to max
	CurrentHP = MaxHP;

	// update the HUD
	OnDamaged.Broadcast(1.0f);

	// Add HUD Locally
	if (IsLocallyControlled())
	{
		// create the UI
		auto* playerController = Cast<APlayerController>(GetOwner());
		//auto* shooterGamemode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (playerController && ShooterUIClass)
		{
			ShooterUI = CreateWidget<UShooterUI>(playerController, ShooterUIClass);
			ShooterUI->AddToViewport(0);
		}
	}
}


void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this,
		                                   &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this,
		                                   &AShooterCharacter::DoSwitchWeapon);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this,
		                                   &AShooterCharacter::ReloadCurrentWeapon);
	}
}



void AShooterCharacter::ReloadCurrentWeapon()
{
	// Default shooter reload implementation
}


void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AShooterCharacter, CurrentWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AShooterCharacter, OwnedWeapons, COND_OwnerOnly);
}


float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser)
{
	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// Reduce HP
	CurrentHP -= Damage;

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return Damage;
}


void AShooterCharacter::DoStartFiring()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	// stop firing the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1)
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else
		{
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

bool AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	if (!HasAuthority()) return false; // Server authorizes weapon addition

	// Do we already own the weapon, reactivate it but dont pickup new weapon 
	if (auto* OwnedWeapon = FindWeaponOfType(WeaponClass); OwnedWeapon && CurrentWeapon)
	{
		CurrentWeapon->DeactivateWeapon();
		CurrentWeapon = OwnedWeapon;
		CurrentWeapon->ActivateWeapon();
		return false;
	}

	// spawn the new weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

	if (AddedWeapon)
	{
		// add the weapon to the owned list
		OwnedWeapons.Add(AddedWeapon);

		// if we have an existing weapon, deactivate it
		if (CurrentWeapon)
		{
			CurrentWeapon->DeactivateWeapon();
		}

		// switch to the new weapon
		CurrentWeapon = AddedWeapon;
		CurrentWeapon->ActivateWeapon();
	}

	return true;
}


void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// unhide this weapon
	Weapon->SetActorHiddenInGame(false);

	if (IsLocallyControlled())
	{
		// Update local client ui 
		OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());
	}

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// ensure we're no longer firing this weapon while deactivated
	Weapon->StopFiring();

	Weapon->StopAltFiring();

	// hide the weapon
	Weapon->SetActorHiddenInGame(true);
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;
}

void AShooterCharacter::Die()
{
	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// disable controls
	DisableInput(nullptr);

	// reset the bullet counter UI
	OnBulletCountUpdated.Broadcast(0, 0);

	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();

	// call the BP handler
	// Actually just contains ragdoll in blueprint, could move to cpp
	BP_OnDeath();

	// schedule character respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}


void AShooterCharacter::OnRespawn()
{
	Destroy();
}
