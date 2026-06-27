// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerGameMode.h"

#include "MultiplayerPlayer.h"
#include "MultiplayerPortalCharacter.h"
#include "Portal.h"
#include "ShooterPickup.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"

void AMultiplayerGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AMultiplayerGameMode::AddKillsAndDeathsToPlayers(AController* KillingPlayer, AController* DyingPlayer)
{
	if (!KillingPlayer && !DyingPlayer) return;
	FMultiplayerPortalPlayerStats* KillingPlayerStats = PlayerMap.Find(KillingPlayer);
	
	// Player who killed is the same as one who died, like in grenade case, only increment death count
	if (KillingPlayerStats && (KillingPlayer == DyingPlayer))
	{
		++KillingPlayerStats->DeathCount;
		if (auto* Character = Cast<AMultiplayerPlayer>(KillingPlayer->GetPawn()))
		{
			Character->HandleNewStatValues(*KillingPlayerStats);
		}
		return;
	}
	
	
	// Split into 2 if statements incase either player logs out prior to propagation
	if (KillingPlayerStats)
	{
		++KillingPlayerStats->KillCount;
		if (auto* Character = Cast<AMultiplayerPlayer>(KillingPlayer->GetPawn()))
		{
			Character->HandleNewStatValues(*KillingPlayerStats);
		}

	}
	
	FMultiplayerPortalPlayerStats* DyingPlayerStats = PlayerMap.Find(DyingPlayer);
	if (DyingPlayerStats)
	{
		++DyingPlayerStats->DeathCount;
		if (auto* Character = Cast<AMultiplayerPlayer>(DyingPlayer->GetPawn()))
		{
			Character->HandleNewStatValues(*DyingPlayerStats);
		}
	}
	

}

bool AMultiplayerGameMode::GetPlayerStats(AController* player, FMultiplayerPortalPlayerStats& outStats)
{
	if (FMultiplayerPortalPlayerStats* playerStats = PlayerMap.Find(player))
	{
		outStats = *playerStats;
		return true;
	}
	return false;
}


void AMultiplayerGameMode::AddPlayerWithPortalGun(AActor* PickingPlayer, AShooterPickup* Pickup)
{
	// Player already has a portal gun, since theyve already picked the second, respawn the first
	// Can be handled by pickup prevention in the pickup class instead
	if (auto* PortalPickup = PlayersWithPortalGun.Find(PickingPlayer))
	{
		// Player is not picking from the same pickup they already did, incase double collision fire or physics misstep
		if (*PortalPickup != Pickup)
		{
			auto* PortalPickupPtr = *PortalPickup;
			PortalPickupPtr->RespawnPickup();
		}
	}
	// Adding also replaces in map, if player already had a pickup, will be replaced with this reference
	PlayersWithPortalGun.Add(PickingPlayer, Pickup);
	PickingPlayer->OnDestroyed.AddUniqueDynamic(this, &AMultiplayerGameMode::PlayerWithPortalGunDestroyed);
}

void AMultiplayerGameMode::PlayerWithPortalGunDestroyed(AActor* ActorToBeDestroyed)
{
	// Pointer-to-Pointer AShooterPickup
	if (auto* shooterPickup = PlayersWithPortalGun.Find(ActorToBeDestroyed))
	{
		// Unbind respawn delegate from actor
		ActorToBeDestroyed->OnDestroyed.RemoveDynamic(this, &AMultiplayerGameMode::PlayerWithPortalGunDestroyed);
		if (auto* multiplayerPlayer = Cast<AMultiplayerPlayer>(ActorToBeDestroyed))
		{
			// Stop player checking if walls allow portals on them
			multiplayerPlayer->StopCheckingForPortalWalls();
		}
		AShooterPickup* pickupPtr = *shooterPickup;
		pickupPtr->RespawnPickup();
		PlayersWithPortalGun.Remove(ActorToBeDestroyed);
	}
}

void AMultiplayerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (auto* Character = Cast<AMultiplayerPlayer>(NewPlayer->GetPawn()))
	{
		if (FMultiplayerPortalPlayerStats* PlayerStats = PlayerMap.Find(NewPlayer))
		{
			Character->HandleNewStatValues(*PlayerStats);
			return;
		}
		
		FMultiplayerPortalPlayerStats Stats;
		// Assign color based on how many players already have colors
		int32 ColorIndex = GetNumPlayers() - 1; // 0-based, first player gets White
		FLinearColor ChosenColor = AvailableColors.IsValidIndex(ColorIndex) ? AvailableColors[ColorIndex]
																			: FLinearColor::White; // If no available colours, assign white
		Stats.PlayerColor = ChosenColor;
		Stats.KillCount = 0;
		Stats.DeathCount = 0;
		
		PlayerMap.Add(NewPlayer, Stats);
		Character->HandleNewStatValues(Stats);
		
	}
}

AActor* AMultiplayerGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	if (PlayerStarts.Num() == 0)
		return Super::ChoosePlayerStart_Implementation(Player);

	// Simple random, for unique spawns per player, should keep a UsedStarts array.
	int32 Index = FMath::RandRange(0, PlayerStarts.Num() - 1);
	return PlayerStarts[Index];
}

