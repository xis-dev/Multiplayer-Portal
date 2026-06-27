// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterBulletCounterUI.generated.h"

/**
 *  Simple bullet counter UI widget for a first person shooter game
 */
UCLASS(abstract)
class MULTIPLAYERPORTAL_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Allows Blueprint to update sub-widgets with the new bullet count */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateBulletCounter"))
	void BP_UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	/** Allows Blueprint to update sub-widgets with the new life total and play a damage effect on the HUD */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "Damaged"))
	void BP_Damaged(float LifePercent);
	
	/** Allows Blueprint to update sub-widgets with the new kill count */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateKillCount"))
	void BP_UpdateKillCount(int32 Kills);
	
	/** Allows Blueprint to update sub-widgets with the new death count */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateDeathCount"))
	void BP_UpdateDeathCount(int32 Deaths);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateSessionID"))
	void BP_UpdateSessionID(const FString& SessionID);
};
