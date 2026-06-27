// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PortalSet.h"
#include "ShooterGameMode.h"
#include "MultiplayerGameMode.generated.h"

class AShooterPickup;
/**
 * 
 */
UCLASS()
class MULTIPLAYERPORTAL_API AMultiplayerGameMode : public AShooterGameMode
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Data")
	TMap<AController*, FMultiplayerPortalPlayerStats> PlayerMap;
	
	// Allows multiple players incase multiple portal guns
	UPROPERTY(VisibleAnywhere, Category = "Portal")
	TMap<AActor*, AShooterPickup*> PlayersWithPortalGun;

	virtual void BeginPlay() override;


public:
	
	UFUNCTION()
	void AddKillsAndDeathsToPlayers(AController* KillingPlayer, AController* DyingPlayer);
	
	// returns true/false if given player's stats were found, outputs stats to outStats
	UFUNCTION()
	bool GetPlayerStats(AController* player, FMultiplayerPortalPlayerStats& outStats);
	
	UFUNCTION()
	void AddPlayerWithPortalGun(AActor* PickingPlayer, AShooterPickup* Pickup);
	
	UFUNCTION()
	void PlayerWithPortalGunDestroyed(AActor* ActorToBeDestroyed);
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	


	// Array of colors: White, Red, Blue, Green
	UPROPERTY(EditDefaultsOnly, Category = "Players")
	TArray<FLinearColor> AvailableColors{FLinearColor::White,
										 FLinearColor::Red,
										 FLinearColor::Blue,
										 FLinearColor::Green,
	};
};
