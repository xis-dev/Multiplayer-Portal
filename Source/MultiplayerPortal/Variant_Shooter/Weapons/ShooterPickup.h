// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "ShooterPickup.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AShooterWeapon;

/**
 *  Holds information about a type of weapon pickup
 */
USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Mesh to display on the pickup */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/** Weapon class to grant on pickup */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AShooterWeapon> WeaponToSpawn;
};

/**
 *  Simple shooter game weapon pickup
 */
UCLASS(abstract)
class MULTIPLAYERPORTAL_API AShooterPickup : public APickup
{
	GENERATED_BODY()

protected:
	/** Data on the type of picked weapon and visuals of this pickup */
	UPROPERTY(EditAnywhere, Category="Pickup")
	FDataTableRowHandle WeaponType;

	/** Type to weapon to grant on pickup. Set from the weapon data table. */
	TSubclassOf<AShooterWeapon> WeaponClass;

	/** Timer to respawn the pickup */
	FTimerHandle RespawnTimer;

public:	
	
	/** Constructor */
	AShooterPickup();

protected:

	/** Native construction script */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Gameplay Initialization*/
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Pickup
	virtual void PickupItem(AActor* PickingActor) override;


};
