// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalProjectile.h"

#include "MultiplayerGameState.h"
#include "Portal.h"
#include "MultiplayerPlayer.h"
#include "PortalWall.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PortalWall.h"
#include "Net/UnrealNetwork.h"

#define PortalWallChannel ECollisionChannel::ECC_GameTraceChannel3

void APortalProjectile::ChangeColour(bool bIsOrange)
{
	bIsOrangePortal = bIsOrange;
}

void APortalProjectile::OnRep_ProjectileColour()
{
	// Init with blue colour
	FLinearColor color{0.117, 0.117, 1.0};
	if (bIsOrangePortal) color =  FLinearColor{1.0, 0.250, 0.008};
	ProjectileMesh->SetColorParameterValueOnMaterials("FlatColor", color);
}


APortalProjectile::APortalProjectile()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	
	HitDamage = 0.0f;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
}

void APortalProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APortalProjectile, bIsOrangePortal);
}

void APortalProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
                                  bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	// Let the base class do its own thing
	
	if (!HasAuthority()) return; // Server authorizes hit
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Make sure we have a valid world and blocking hit
	if (!GetWorld() || !Hit.bBlockingHit)
		return;

	// Ignore hits against the firing player
	if (Other == GetInstigator())
		return;


	const FVector HitLoc = Hit.ImpactPoint + (Hit.ImpactNormal * 7.5f); // Offset hit a bit
	const FVector HitNorm = Hit.ImpactNormal;

	if (CanPlacePortal(HitLoc, HitNorm, Other, OtherComp))
	{
		
		if (auto* GameState = Cast<AMultiplayerGameState>(UGameplayStatics::GetGameState(GetWorld())))
		{	
			if (!GameState->TrySpawnPortalWithColor(HitLoc, HitNorm, bIsOrangePortal))
			{
				UE_LOG(LogTemp, Warning, TEXT("Portal projectile failed to spawn portal"));
			}
		}
	}
	
	
	// Destroy the projectile
	Destroy();
}

bool APortalProjectile::CanPlacePortal(const FVector& Location, const FVector& Normal, AActor* HitActor, UPrimitiveComponent* HitComp) const
{
	// Only allow portals on actors tagged as "PortalWall"
	if ((HitActor && HitActor->ActorHasTag("PortalWall")) || (HitComp && HitComp->GetCollisionObjectType() == PortalWallChannel))
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal placement allowed on PortalWall!"));
		return true;	
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Cannot place portal: actor is not a PortalWall"));
	return false;
}

