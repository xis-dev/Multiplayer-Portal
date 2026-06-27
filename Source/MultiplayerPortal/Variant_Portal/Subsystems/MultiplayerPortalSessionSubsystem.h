// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FindSearchResults.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JoinSessionResult.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerPortalSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortalOnCreateSessionComplete, bool, Success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortalOnUpdateSessionComplete, bool, Success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortalOnStartSessionComplete, bool, Success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortalOnEndSessionComplete, bool, Success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortalOnDestroySessionComplete, bool, Success);




DECLARE_MULTICAST_DELEGATE_TwoParams(FPortalOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool Success);
// Delegate to expose find session result to UE wrapped types(UCLASS, USTRUCT...)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPortalOnFindSessionsComplete_Exposed, TArray<FindSearchResults>, SessionResults, bool, Success); 

DECLARE_MULTICAST_DELEGATE_OneParam(FPortalOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
// Delegate to expose join session result to UE wrapped types(UCLASS, USTRUCT...)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortalOnJoinSessionComplete_Exposed, FJoinSessionResult, Result);
UCLASS()
class MULTIPLAYERPORTAL_API UMultiplayerPortalSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerPortalSessionSubsystem();
	
	// Current session id after joining/creating a session
	UPROPERTY()
	FString CurrentSessionID;
	
	void CreateSession(int32 NumPublicConnections, bool IsLAN, FString LevelName);
	FPortalOnCreateSessionComplete OnCreateSessionComplete_Event;
	
	void UpdateSession();
	FPortalOnUpdateSessionComplete OnUpdateSessionComplete_Event;
	
	void StartSession();
	FPortalOnStartSessionComplete OnStartSessionComplete_Event;
	
	void EndSession();
	FPortalOnEndSessionComplete OnEndSessionComplete_Event;
	
	void DestroySession();
	FPortalOnDestroySessionComplete OnDestroySessionComplete_Event;
	
	void FindSessions(int32 MaxSearchResults, bool IsLANQuery);
	FPortalOnFindSessionsComplete OnFindSessionsComplete_Event;
	FPortalOnFindSessionsComplete_Exposed OnFindSessionsComplete_Exposed;
	
	void JoinGameSession(const FOnlineSessionSearchResult& SessionResult);
	FPortalOnJoinSessionComplete OnJoinGameSessionComplete_Event;
	FPortalOnJoinSessionComplete_Exposed OnJoinSessionsComplete_Exposed;
protected:
	
	void OnCreateSessionCompleted(FName SessionName, bool Success);
	
	void OnUpdateSessionCompleted(FName SessionName, bool Success);
	
	void OnStartSessionCompleted(FName SessionName, bool Success);
	
	void OnEndSessionCompleted(FName SessionName, bool Success);
	
	void OnDestroySessionCompleted(FName SessionName, bool Success);
	
	void OnFindSessionsCompleted(bool Success);
	
	void OnSessionsFound(const TArray<FOnlineSessionSearchResult>& SessionResults, bool Success);
	
	
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionJoined(EOnJoinSessionCompleteResult::Type Result);
	bool TryTravelToCurrentSession();
	

private:
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	
	// Session Creation
	
	FOnCreateSessionCompleteDelegate CreateSessionComplete_Delegate;
	FDelegateHandle CreateSessionComplete_DelegateHandle;
	
	// Session Update
	
	FOnUpdateSessionCompleteDelegate UpdateSessionComplete_Delegate;
	FDelegateHandle UpdateSessionComplete_DelegateHandle;
	
	// Session Start
	
	FOnStartSessionCompleteDelegate StartSessionComplete_Delegate;
	FDelegateHandle StartSessionComplete_DelegateHandle;
	
	// Session End
	
	FOnEndSessionCompleteDelegate EndSessionComplete_Delegate;
	FDelegateHandle EndSessionComplete_DelegateHandle;
	
	// Session Destroy
	
	FOnDestroySessionCompleteDelegate DestroySessionComplete_Delegate;
	FDelegateHandle DestroySessionComplete_DelegateHandle;
	
	// Sessions Find
	
	FOnFindSessionsCompleteDelegate FindSessionsComplete_Delegate;
	FDelegateHandle FindSessionsComplete_DelegateHandle;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	
	// Session Join
	
	FOnJoinSessionCompleteDelegate JoinSessionComplete_Delegate;
	FDelegateHandle JoinSessionComplete_DelegateHandle;
};
