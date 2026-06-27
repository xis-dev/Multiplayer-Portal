// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Rifle.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPORTAL_API ARifle : public AShooterWeapon
{
	GENERATED_BODY()
	
public:
	ARifle()
	{
		bUseLineTrace = true;
		LineTraceDamage = 20.0f;
		RefireRate = 0.2f;
		HeadshotMultiplier = 100.0f;  // On shot kill for Headshots for now	
		MagazineSize = 10;
		bFullAuto = true;   // Rifles should Autofire
	}
	
};
