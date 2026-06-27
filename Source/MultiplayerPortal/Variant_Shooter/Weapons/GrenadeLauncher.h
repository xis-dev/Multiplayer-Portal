// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "GrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPORTAL_API AGrenadeLauncher : public AShooterWeapon
{
	GENERATED_BODY()
	
	
public:
	AGrenadeLauncher()
	{
		bUseLineTrace = false;
		RefireRate = 5.0f;
		MagazineSize = 5;
		bFullAuto = false;
		
		RadialDamageRadius = 400.0f;
	}
protected:
	// Override projectile OnHit to apply radial damage
	virtual AShooterProjectile* FireProjectile(const FVector& TargetLocation) override
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, FString::Printf(TEXT("Fire projectile is being run on: %s"), *GetWorld()->GetPackage()->GetName()));

		return Super::FireProjectile(TargetLocation);
	}
	
};
