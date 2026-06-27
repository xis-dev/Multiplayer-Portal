// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeProjectile.h"


// Sets default values
AGrenadeProjectile::AGrenadeProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	HitDamage = 125.0f;
}

// Called when the game starts or when spawned
void AGrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrenadeProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

