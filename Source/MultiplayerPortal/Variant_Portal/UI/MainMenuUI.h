// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/FindSearchResults.h"
#include "Subsystems/JoinSessionResult.h"
#include "MainMenuUI.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPORTAL_API UMainMenuUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create Session")) 
	void CreateSession();
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Join Session")) 
	void TryJoinSession(const FString& id);
	
	UFUNCTION()
	void OnSessionCreated(bool Success);
	
	UFUNCTION()
	void OnFoundSessions(TArray<FindSearchResults> SessionResults, bool Success);
	
	UPROPERTY()
	TMap<FString, FindSearchResults> IDsAndSessions;
	
	UPROPERTY()
	FString CurrentJoiningID;
	
	// Util function to get random string when generating session ids
	UFUNCTION(Category = "Utility")
	FString GetRandomString(int32 length);
	
	
	UFUNCTION(BlueprintNativeEvent)
	void OnJoinedSession(FJoinSessionResult JoinResult);
};
