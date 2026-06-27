// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterPlayerController.h"
#include "MultiplayerPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPORTAL_API AMultiplayerPlayerController : public AShooterPlayerController
{
	GENERATED_BODY()

public:
	
protected:
	
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	// Client "OnPossess", Acknowledges Pawn Posession on client machine
	virtual void AcknowledgePossession(class APawn* P) override;
	
	virtual void OnPawnDestroyed(AActor* DestroyedActor) override;
	
	UFUNCTION(Server, Reliable)
	void Server_OnPawnDestroyed();
	
	virtual void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets) override;
	
	
	virtual void OnPawnDamaged(float LifePercent) override;
	
	UFUNCTION()
	virtual void OnKillCountUpdated(int32 Kills);
	
	UFUNCTION()
	virtual void OnDeathCountUpdated(int32 Deaths);
	
	UFUNCTION()
	virtual void UpdateSessionID(FString SessionID);
	
	
};
