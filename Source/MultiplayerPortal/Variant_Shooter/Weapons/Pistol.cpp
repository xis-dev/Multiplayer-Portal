// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/Weapons/Pistol.h"

APistol::APistol()
{
	bUseLineTrace = true;
	LineTraceDamage = 15.0f;
	HeadshotMultiplier = 100.0f;  // On shot kill for Headshots for now	
	RefireRate = 2.0f;
	MagazineSize = 6;
	bFullAuto = false;
}
