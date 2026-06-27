// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterPickup.h"

#include "MultiplayerGameMode.h"
#include "MultiplayerPlayer.h"
#include "Portal.h"
#include "PortalGun.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShooterWeaponHolder.h"
#include "ShooterWeapon.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AShooterPickup::AShooterPickup()
{
 	PrimaryActorTick.bCanEverTick = true;
}

void AShooterPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (FWeaponTableRow* WeaponData = WeaponType.GetRow<FWeaponTableRow>(FString()))
	{
		// set the mesh
		Mesh->SetStaticMesh(WeaponData->StaticMesh.LoadSynchronous());
	}
}

void AShooterPickup::BeginPlay()
{
	Super::BeginPlay();

	if (FWeaponTableRow* WeaponData = WeaponType.GetRow<FWeaponTableRow>(FString()))
	{
		// copy the weapon class
		WeaponClass = WeaponData->WeaponToSpawn;
	}
}

void AShooterPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterPickup::PickupItem(AActor* PickingActor)
{
	// have we collided against a weapon holder?
	if (IShooterWeaponHolder* WeaponHolder = Cast<IShooterWeaponHolder>(PickingActor))
	{
		if (!WeaponHolder->AddWeaponClass(WeaponClass)) return;
		
		// Different implementation for portal guns
		if (WeaponClass->IsChildOf(APortalGun::StaticClass()))
		{
			if (!HasAuthority()) return;
			
			if (auto* gameMode = Cast<AMultiplayerGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				gameMode->AddPlayerWithPortalGun(PickingActor, this);
			}
			
			
			// What the original pickup item would have done, bar from starting a respawn timer
			// hide this mesh
			SetActorHiddenInGame(true);

			// disable collision
			SetActorEnableCollision(false);

			// disable tickings
			SetActorTickEnabled(false);
			
			if (auto* multiplayerPlayer  = Cast<AMultiplayerPlayer>(WeaponHolder))
			{
				multiplayerPlayer->StartCheckingForPortalWalls();
			}
			
			return;
		}
		
		Super::PickupItem(PickingActor);
	}
	
}




