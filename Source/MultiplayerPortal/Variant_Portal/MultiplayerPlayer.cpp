#include "MultiplayerPlayer.h"

#include "EnhancedInputComponent.h"
#include "MultiplayerGameMode.h"
#include "MultiplayerGameState.h"
#include "MultiplayerPortalPlayerStats.h"
#include "ShooterWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Portal/Portal.h"
#include "Portal/PortalPoint.h"

#define PortalWallObjectChannel ECC_GameTraceChannel3
#define ShootableTraceChannel ECC_GameTraceChannel2
struct FPortalPoint;

AMultiplayerPlayer::AMultiplayerPlayer()
{
	// Setup head collider and attachments
	HeadCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadCollider"));
	BodyCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollider"));
	BodyCollider->SetupAttachment(RootComponent);
	if (auto* mesh = GetMesh())
	{
		BodyCollider->SetupAttachment(mesh, BodySocket);
		HeadCollider->SetupAttachment(mesh, HeadSocket);
		HeadCollider->ComponentTags.Add(FName("PlayerHead"));

		BodyCollider->SetCollisionResponseToAllChannels(ECR_Block);
		HeadCollider->SetCollisionResponseToAllChannels(ECR_Block);
		
		BodyCollider->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		HeadCollider->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		
	}
	if (auto* capsuleComp = GetCapsuleComponent())
	{
		capsuleComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		capsuleComp->SetCollisionResponseToChannel(ShootableTraceChannel, ECR_Ignore);
	}
}

void AMultiplayerPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMultiplayerPlayer, PlayerColor);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayer, Kills, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayer, Deaths, COND_OwnerOnly);
}

void AMultiplayerPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMultiplayerPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Alt Firing
		EnhancedInputComponent->BindAction(AltFireAction, ETriggerEvent::Started, this,
		                                   &AMultiplayerPlayer::DoStartAltFiring);
		EnhancedInputComponent->BindAction(AltFireAction, ETriggerEvent::Completed, this,
		                                   &AMultiplayerPlayer::DoStopAltFiring);
		
		// Weapon switching
		EnhancedInputComponent->BindAction(PrevWeaponAction, ETriggerEvent::Started, this,
								   &AMultiplayerPlayer::SwitchToPreviousWeapon);
		EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Started, this,
								   &AMultiplayerPlayer::SwitchToNextWeapon);
	}
}


void AMultiplayerPlayer::OnRep_PlayerColor()
{
	// Apply color to the character mesh material
	USkeletalMeshComponent* CharMesh = GetMesh();
	USkeletalMeshComponent* FPSMesh = GetFirstPersonMesh();
	if (CharMesh && FPSMesh)
	{
		// Player mesh has 2 materials
		UMaterialInstanceDynamic* DynMat0_Char = CharMesh->CreateAndSetMaterialInstanceDynamic(0);
		UMaterialInstanceDynamic* DynMat0_FPS = FPSMesh->CreateAndSetMaterialInstanceDynamic(0);
		UMaterialInstanceDynamic* DynMat1_Char = CharMesh->CreateAndSetMaterialInstanceDynamic(1);
		UMaterialInstanceDynamic* DynMat1_FPS = FPSMesh->CreateAndSetMaterialInstanceDynamic(1);

		if (DynMat0_Char && DynMat0_FPS && DynMat1_Char && DynMat1_FPS)
		{
			DynMat0_Char->SetVectorParameterValue("BodyColor", PlayerColor);
			DynMat0_FPS->SetVectorParameterValue("BodyColor", PlayerColor);
			DynMat1_Char->SetVectorParameterValue("BodyColor", PlayerColor);
			DynMat1_FPS->SetVectorParameterValue("BodyColor", PlayerColor);
		}
	}
}

void AMultiplayerPlayer::OnRep_Kills()
{
	if (IsLocallyControlled())
	{
		OnKillCountUpdated.Broadcast(Kills);
	}
	// Update kills ui
}

void AMultiplayerPlayer::OnRep_Deaths()
{
	if (IsLocallyControlled())
	{
		OnDeathCountUpdated.Broadcast(Deaths);
	}
	// Update deaths ui
}


// Dont need ShooterCharacter implementation, doesnt consider multiplayer
void AMultiplayerPlayer::ReloadCurrentWeapon()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		Server_ReloadWeapon();
	}
}

void AMultiplayerPlayer::Server_ReloadWeapon_Implementation()
{
	TryReloadWeapon_Server();
}

bool AMultiplayerPlayer::TryReloadWeapon_Server()
{
	if (!HasAuthority() || !CurrentWeapon) return false;

	int32 Needed = CurrentWeapon->GetMagazineSize() - CurrentWeapon->GetBulletCount();
	if (Needed <= 0) return false;

	CurrentWeapon->AddBullets(Needed);
	return true;
}

void AMultiplayerPlayer::SwitchToPreviousWeapon()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		Server_SwitchToPreviousWeapon();
	}
}

void AMultiplayerPlayer::Server_SwitchToPreviousWeapon_Implementation()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1)
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the first weapon?
		if (WeaponIndex == 0)
		{
			// loop back to the end of the array
			WeaponIndex = OwnedWeapons.Num() - 1;
		}
		else
		{
			// select the previous weapon index
			--WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}	
}


void AMultiplayerPlayer::SwitchToNextWeapon()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		Server_SwitchToNextWeapon();
	}
}

void AMultiplayerPlayer::Server_SwitchToNextWeapon_Implementation()
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



void AMultiplayerPlayer::CheckSpawnableWall()
{
	if (!IsLocallyControlled()) return;
	FHitResult hit;

	FVector start = GetFirstPersonCameraComponent()->GetComponentLocation();
	FVector end = start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);
	if (GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Visibility))
	{
		if (hit.Component->GetCollisionObjectType() == PortalWallObjectChannel)
		{
			CanSeeValidWall();
			return;
		}
	}

	CannotSeeValidWall();
}

void AMultiplayerPlayer::CanSeeValidWall_Implementation()
{
}

void AMultiplayerPlayer::CannotSeeValidWall_Implementation()
{
}

void AMultiplayerPlayer::Client_OnDamageTaken_Implementation(float CurrentHealth, float MaxHealth)
{
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHealth / MaxHealth));
}

float AMultiplayerPlayer::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                     class AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.0; // Server only should handle take damage
	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// Reduce HP
	CurrentHP -= DamageAmount;
	Client_OnDamageTaken(CurrentHP, MaxHP);

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		if (auto* gameMode = Cast<AMultiplayerGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			gameMode->AddKillsAndDeathsToPlayers(EventInstigator, GetController());
		}
		Die();
	}

	return DamageAmount;
}

// Dont need the ShooterCharacter implementation, doesnt take multiplayer into account
void AMultiplayerPlayer::DoStartFiring()
{
	// Client propagates to server to start firing
	if (IsLocallyControlled()) // May not be needed? input is local anyway
	{
		ServerStartFiring();
	}
}

// Dont need the ShooterCharacter implementation, doesnt take multiplayer into account
void AMultiplayerPlayer::DoStopFiring()
{
	// Client propagates to server to stop firing
	if (IsLocallyControlled()) // May not be needed? input is local anyway
	{
		ServerStopFiring();
	}
}

void AMultiplayerPlayer::ServerStartFiring_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFiring();
	}
}

void AMultiplayerPlayer::ServerStopFiring_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AMultiplayerPlayer::ServerStartAltFiring_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartAltFiring();
	}
}

void AMultiplayerPlayer::ServerStopAltFiring_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopAltFiring();
	}
}

void AMultiplayerPlayer::DoStartAltFiring()
{
	// Client propagates to server to start firing
	if (IsLocallyControlled()) // May not be needed? input is local anyway
	{
		ServerStartAltFiring();
	}
}

void AMultiplayerPlayer::DoStopAltFiring()
{
	// Client propagates to server to stop firing
	if (IsLocallyControlled()) // May not be needed? input is local anyway
	{
		ServerStopAltFiring();
	}
}

// Dont need ShooterChar die implementation, doesnt take into account multiplayer
void AMultiplayerPlayer::DoSwitchWeapon()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		ServerSwitchWeapon();
	}
}

void AMultiplayerPlayer::ServerSwitchWeapon_Implementation()
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

void AMultiplayerPlayer::HandleNewStatValues(FMultiplayerPortalPlayerStats stats)
{
	if (IsLocallyControlled() || HasAuthority()) // Only owning client and server can try to handle stat values
	{
		Server_HandleNewStatValues(stats);
	}
}

void AMultiplayerPlayer::Server_HandleNewStatValues_Implementation(FMultiplayerPortalPlayerStats stats)
{
	SetPlayerColor(stats.PlayerColor);

	// Will replicate to owning player to handle ui changes
	Kills = stats.KillCount;
	OnRep_Kills();
	Deaths = stats.DeathCount;
	OnRep_Deaths();
}


void AMultiplayerPlayer::SetPlayerColor(FLinearColor NewColor)
{
	if (HasAuthority())
	{
		PlayerColor = NewColor;
		OnRep_PlayerColor(); // Apply on server
	}
}

void AMultiplayerPlayer::HasBeenTeleported()
{
	ITeleportable::HasBeenTeleported();
	CanTeleport = false;
	GetWorld()->GetTimerManager().SetTimer(TeleportHandle, this, &AMultiplayerPlayer::ResetCanTeleport, 0.25f);
}

bool AMultiplayerPlayer::GetCanTeleport()
{
	return CanTeleport;
}

void AMultiplayerPlayer::ResetCanTeleport()
{
	ITeleportable::ResetCanTeleport();
	CanTeleport = true;
}

void AMultiplayerPlayer::StartCheckingForPortalWalls()
{
	GetWorld()->GetTimerManager().SetTimer(WallCheckHandle, this, &AMultiplayerPlayer::CheckSpawnableWall, 0.1f, true);
}

void AMultiplayerPlayer::StopCheckingForPortalWalls()
{
	GetWorld()->GetTimerManager().ClearTimer(WallCheckHandle);
}


// Dont need ShooterChar die implementation, doesnt take into account multiplayer
void AMultiplayerPlayer::Die()
{
	// Server multicast 
	MulticastDeath();

}

void AMultiplayerPlayer::MulticastDeath_Implementation()
{
	// Owning client only death implementation(UI & Input)
	if (IsLocallyControlled())
	{
		// disable controls
		DisableInput(nullptr);

		// reset the bullet counter UI
		OnBulletCountUpdated.Broadcast(0, 0);
	}
	
	if (HasAuthority())
	{
		// deactivate the weapon
		if (IsValid(CurrentWeapon))
		{
			CurrentWeapon->DeactivateWeapon();
		}
		// schedule character respawn
		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AMultiplayerPlayer::OnRespawn, RespawnTime, false);
	}
	
	// stop character movement on all machines
	GetCharacterMovement()->StopMovementImmediately();
    
	// call the BP handler
	// Actually just contains ragdoll
	BP_OnDeath();
	
}

void AMultiplayerPlayer::OnRespawn()
{
	Super::OnRespawn();
}
