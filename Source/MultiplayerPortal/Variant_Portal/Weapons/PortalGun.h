// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterProjectile.h"
#include "ShooterWeapon.h"
#include "PortalGun.generated.h"

UCLASS()
class MULTIPLAYERPORTAL_API APortalGun : public AShooterWeapon
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APortalGun();
	
	

protected:
	
	// What colour of projectile is going to be spawned
	UPROPERTY()
	bool ProjectileToSpawnIsOrange;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnRep_CurrentBullets() override;
	
	virtual void Fire() override;
	
	virtual void ServerFire() override;
	
	virtual void ExecuteFire() override;
	
	virtual void AltFire() override;
	
	virtual void ExecuteAltFire() override;
	
	// Shoot portal projectile based on if its orange or not, only 2 portal states, blue or orange
	void ShootPortalProjectile(bool isOrange);
	
	virtual AShooterProjectile* FireProjectile(const FVector& TargetLocation) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
