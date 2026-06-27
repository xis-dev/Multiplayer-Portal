// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerGameState.h"

#include "MultiplayerPlayer.h"
#include "Portal.h"
#include "PortalPoint.h"
#include "PortalSet.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"



#define PortalWallObjectChannel ECC_GameTraceChannel3

void AMultiplayerGameState::BeginPlay()
{
	Super::BeginPlay();
	
	GetDefaultPortalBounds(PortalBoundsMin, PortalBoundsMax);
}

void AMultiplayerGameState::OnRep_Portals()
{
	// Set local player's portal parameters for each portal
	if (Portals.BluePortal && Portals.OrangePortal)
	{
		Portals.OrangePortal->PortalSpawned(Portals.BluePortal, true);
		Portals.BluePortal->PortalSpawned(Portals.OrangePortal, false);
	}
	else if (Portals.OrangePortal && !Portals.BluePortal)
	{
		Portals.OrangePortal->PortalSpawned(nullptr, true);
	}
	else if (Portals.BluePortal && !Portals.OrangePortal)
	{
		Portals.BluePortal->PortalSpawned(nullptr, false);
	}
}

void AMultiplayerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMultiplayerGameState, Portals);
}

void AMultiplayerGameState::SetPortalLocalPlayer_Implementation(APawn* playerPawn)
{
	// Set Portal's Local player for independent scene capture views
	auto* mPawn = Cast<AMultiplayerPlayer>(playerPawn);
	if (Portals.OrangePortal)
	{
		Portals.OrangePortal->SetLocalPlayer(mPawn);
	}
	if (Portals.BluePortal)
	{
		Portals.BluePortal->SetLocalPlayer(mPawn);
	}
}



void AMultiplayerGameState::GetDefaultPortalBounds(FVector& Min, FVector& Max)
{
	// Spawn a temporary portal to get the size of portals for testing
	if (auto* tempPortal = GetWorld()->SpawnActor<APortal>(PortalClass, FVector(999999999.0f), FRotator(0.0f)))
	{
		tempPortal->FinalPortalMesh->GetLocalBounds(Min, Max);
		FTransform rot = FTransform{tempPortal->FinalPortalMesh->GetRelativeRotation()};
		rot.SetScale3D(tempPortal->FinalPortalMesh->GetRelativeScale3D());
		Min = rot.TransformVector(Min);
		Max = rot.TransformVector(Max);
		tempPortal->Destroy();
		return;
	}
	Min = Max = FVector(0.0f);
}

bool AMultiplayerGameState::CanPortalSpawn(FVector location, FVector impactNormal, FVector& outLoc,
	int32 recDepth, int32 maxRecDepth)
{
		if (recDepth >= maxRecDepth) return false;
	
	// How far should the point check into
	const float inputDistance = 25.0f;
	// How far should the point start out of the normal
	const float outputDistance = 5.0f;
	
	FHitResult hitCenter;
	FVector start = location + (impactNormal * outputDistance);
	FVector end = location +(-impactNormal * (inputDistance + outputDistance)); 
	outLoc = location;
	
	// Center hit nothing
	if (!GetWorld()->LineTraceSingleByChannel(hitCenter,start,end, ECC_Visibility))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot create portal here.")); 
		return false;
	}
	
	// Center hit but invalid wall
	if (hitCenter.Component->GetCollisionObjectType() != PortalWallObjectChannel)
	{
		return false;
	}
	
	FRotator portalRot = APortal::GetRotatorFromNormal(impactNormal);
	
	FVector up = UKismetMathLibrary::GetUpVector(portalRot);
	FVector right = UKismetMathLibrary::GetRightVector(portalRot);
	TArray<FPortalPoint> PointsToCheck;
	
	PointsToCheck.Add({location + up * PortalBoundsMin.Z, up * PortalBoundsMin.Z});
	PointsToCheck.Add({location + up * PortalBoundsMax.Z, up * PortalBoundsMax.Z});
	PointsToCheck.Add({location + right * PortalBoundsMin.Y, right * PortalBoundsMin.Y});
	PointsToCheck.Add({location + right * PortalBoundsMax.Y, right * PortalBoundsMax.Y});
	
	
		for (const FPortalPoint& point: PointsToCheck)
		{

			FVector startPos = point.WorldPos + (impactNormal * outputDistance);
			FVector endPos = startPos +(-impactNormal * (inputDistance + outputDistance)); 
		
			FHitResult hit;
			bool validPoint = false;

			if (GetWorld()->LineTraceSingleByChannel(hit, startPos, endPos, ECC_Visibility))
			{
				// Point hit valid wall with a similar impact normal
				if (hit.Component->GetCollisionObjectType() == PortalWallObjectChannel && hit.ImpactNormal.Equals(impactNormal, 0.1f))
				{
					validPoint = true;
					DrawDebugLine(GetWorld(), startPos, hit.Location, FColor(0, 255, 0), false, 2.0f, 0, 5.0f);
				}
				
			}
			// Point hit nothing
			if (!validPoint)
			{
				FVector pointToCenter = location - point.WorldPos;
				// Move center by the distance of the failed point in the opposite dir and check again
				return CanPortalSpawn(location + pointToCenter, impactNormal, outLoc, recDepth + 1);
			}

		}
	
	outLoc = location;
	return true;

}

bool AMultiplayerGameState::TrySpawnPortalWithColor(FVector hitPos, FVector impactNorm,
                                                 bool isPortalOrange)
{
	if (!HasAuthority()) return false;
	FVector finalSpawnLocation{};
	if (!CanPortalSpawn(hitPos, impactNorm, finalSpawnLocation)) return false;
	
	auto portalRot = APortal::GetRotatorFromNormal(impactNorm);
	
	// Move Orange portal to new location if it already exists
	if (isPortalOrange && Portals.OrangePortal)
	{
		Portals.OrangePortal->SetActorLocationAndRotation(finalSpawnLocation, portalRot);
	}
	else if (!isPortalOrange && Portals.BluePortal) // Move blue portal to new location if it already exists
	{
		Portals.BluePortal->SetActorLocationAndRotation(finalSpawnLocation, portalRot);
	}
	else // If the associated portal did not exist, spawn a new one
	{
		if (auto* portal = GetWorld()->SpawnActor<APortal>(PortalClass, finalSpawnLocation, portalRot))
		{
			if (isPortalOrange)
			{
				Portals.OrangePortal = portal;
			}
			else
			{
				Portals.BluePortal = portal;
			}
			OnRep_Portals(); // update portals on server
			return true;
		}
	}

	
	 return false;
}
