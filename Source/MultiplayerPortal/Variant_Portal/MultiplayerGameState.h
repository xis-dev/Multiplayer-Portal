// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PortalSet.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPORTAL_API AMultiplayerGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Portal")
	TSubclassOf<APortal> PortalClass;
	
	UPROPERTY(ReplicatedUsing = OnRep_Portals)
	FPortalSet Portals;
	
	UPROPERTY()
	FVector PortalBoundsMin;
	
	UPROPERTY()
	FVector PortalBoundsMax;
	
	UFUNCTION()
	void OnRep_Portals();

	
public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
	// Client specifier not needed since it runs on owning client only, wont spend traffic since its already on client but as a safeguard
	UFUNCTION(Client, Reliable)
	void SetPortalLocalPlayer(APawn* Pawn);
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void GetDefaultPortalBounds(FVector& Min, FVector& Max);
	
	UFUNCTION(BlueprintCallable, Category = "Utility")
	bool CanPortalSpawn(FVector location, FVector impactNormal, FVector& outLoc, int32 recDepth = 0, int32 maxRecDepth = 16);
	
	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool TrySpawnPortalWithColor(FVector hitPos, FVector impactNorm, bool isPortalOrange);
	
};
