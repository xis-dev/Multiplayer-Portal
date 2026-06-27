// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerPortalSessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"


UMultiplayerPortalSessionSubsystem::UMultiplayerPortalSessionSubsystem()
	:CreateSessionComplete_Delegate(FOnCreateSessionCompleteDelegate::CreateUObject
									(this, &ThisClass::OnCreateSessionCompleted)),

	UpdateSessionComplete_Delegate(FOnUpdateSessionCompleteDelegate::CreateUObject
									(this, &ThisClass::OnUpdateSessionCompleted)),
									
	StartSessionComplete_Delegate(FOnStartSessionCompleteDelegate::CreateUObject
								(this, &ThisClass::OnStartSessionCompleted)),

	EndSessionComplete_Delegate(FOnEndSessionCompleteDelegate::CreateUObject
							(this, &ThisClass::OnEndSessionCompleted)),

	DestroySessionComplete_Delegate(FOnDestroySessionCompleteDelegate::CreateUObject
						(this, &ThisClass::OnDestroySessionCompleted)),

	FindSessionsComplete_Delegate(FOnFindSessionsCompleteDelegate::CreateUObject
					(this, &ThisClass::OnFindSessionsCompleted)),

	JoinSessionComplete_Delegate(FOnJoinSessionCompleteDelegate::CreateUObject
				(this, &ThisClass::OnJoinSessionCompleted))

{
	
	OnFindSessionsComplete_Event.AddUObject(this, &UMultiplayerPortalSessionSubsystem::OnSessionsFound);
	OnJoinGameSessionComplete_Event.AddUObject(this, &UMultiplayerPortalSessionSubsystem::OnSessionJoined);
}

void UMultiplayerPortalSessionSubsystem::CreateSession(int32 NumPublicConnections, bool IsLAN, FString LevelName)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!sessionInterface.IsValid())
	{
		OnCreateSessionComplete_Event.Broadcast(false);
		return;
	}
	
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bIsLANMatch = IsLAN;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;	
	
	LastSessionSettings->Set(SETTING_MAPNAME, LevelName, EOnlineDataAdvertisementType::ViaOnlineService);
	
	CreateSessionComplete_DelegateHandle = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionComplete_Delegate);
	
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), 
										 NAME_GameSession, 
										 *LastSessionSettings))
	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionComplete_DelegateHandle);
		
		OnCreateSessionComplete_Event.Broadcast(false);
	}
	else // Session successfully created, get session id and set it
	{
		sessionInterface->GetSessionSettings(NAME_GameSession)->Get(SETTING_MAPNAME, CurrentSessionID);
	}
}

void UMultiplayerPortalSessionSubsystem::UpdateSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!sessionInterface.IsValid())
	{
		OnUpdateSessionComplete_Event.Broadcast(false);
		return;
	}
	
	TSharedPtr<FOnlineSessionSettings> updatedSessionSettings = MakeShareable(new FOnlineSessionSettings(*LastSessionSettings));
	
	updatedSessionSettings->Set(SETTING_MAPNAME, FString("Updated Level Name"), EOnlineDataAdvertisementType::ViaOnlineService);
	
	UpdateSessionComplete_DelegateHandle = sessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(UpdateSessionComplete_Delegate);
	
	if (!sessionInterface->UpdateSession(NAME_GameSession, *updatedSessionSettings))
	{
		sessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionComplete_DelegateHandle);
		
		OnUpdateSessionComplete_Event.Broadcast(false);
	}
	else
	{
		LastSessionSettings = updatedSessionSettings;
	}
}

void UMultiplayerPortalSessionSubsystem::StartSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!sessionInterface.IsValid())
	{
		OnStartSessionComplete_Event.Broadcast(false);
		return;
	}
	
	
	StartSessionComplete_DelegateHandle = sessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionComplete_Delegate);
	
	if (!sessionInterface->StartSession(NAME_GameSession))
	{
		sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionComplete_DelegateHandle);
		
		OnStartSessionComplete_Event.Broadcast(false);
	}
}

void UMultiplayerPortalSessionSubsystem::EndSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!sessionInterface.IsValid())
	{
		OnEndSessionComplete_Event.Broadcast(false);
		return;
	}
	
	
	EndSessionComplete_DelegateHandle = sessionInterface->AddOnEndSessionCompleteDelegate_Handle(EndSessionComplete_Delegate);
	
	if (!sessionInterface->EndSession(NAME_GameSession))
	{
		sessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionComplete_DelegateHandle);
		
		OnEndSessionComplete_Event.Broadcast(false);
	}
}

void UMultiplayerPortalSessionSubsystem::DestroySession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!sessionInterface.IsValid())
	{
		OnDestroySessionComplete_Event.Broadcast(false);
		return;
	}
	
	
	DestroySessionComplete_DelegateHandle = sessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionComplete_Delegate);
	
	if (!sessionInterface->DestroySession(NAME_GameSession))
	{
		sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionComplete_DelegateHandle);
		
		OnDestroySessionComplete_Event.Broadcast(false);
	}
}

void UMultiplayerPortalSessionSubsystem::FindSessions(int32 MaxSearchResults, bool IsLANQuery)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!sessionInterface.IsValid())
	{
		OnFindSessionsComplete_Event.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	
	FindSessionsComplete_DelegateHandle = sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsComplete_Delegate);
	
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IsLANQuery;
	
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsComplete_DelegateHandle);
		
		OnFindSessionsComplete_Event.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerPortalSessionSubsystem::JoinGameSession(const FOnlineSessionSearchResult& SessionResult)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!sessionInterface.IsValid())
	{
		OnJoinGameSessionComplete_Event.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	
	JoinSessionComplete_DelegateHandle = sessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionComplete_Delegate);
	
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionComplete_DelegateHandle);
		
		OnJoinGameSessionComplete_Event.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}


void UMultiplayerPortalSessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool Success)
{
	if (const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionComplete_DelegateHandle);
	}
	
	OnCreateSessionComplete_Event.Broadcast(Success);
}

void UMultiplayerPortalSessionSubsystem::OnUpdateSessionCompleted(FName SessionName, bool Success)
{
	if (const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		sessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionComplete_DelegateHandle);
	}
	
	OnUpdateSessionComplete_Event.Broadcast(Success);
}

void UMultiplayerPortalSessionSubsystem::OnStartSessionCompleted(FName SessionName, bool Success)
{
	if (const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionComplete_DelegateHandle);
	}
	
	OnStartSessionComplete_Event.Broadcast(Success);
}

void UMultiplayerPortalSessionSubsystem::OnEndSessionCompleted(FName SessionName, bool Success)
{
	if (const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		sessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionComplete_DelegateHandle);
	}
	
	OnEndSessionComplete_Event.Broadcast(Success);
}

void UMultiplayerPortalSessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool Success)
{
	if (const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionComplete_DelegateHandle);
	}
	
	OnDestroySessionComplete_Event.Broadcast(Success);
}

void UMultiplayerPortalSessionSubsystem::OnFindSessionsCompleted(bool Success)
{
	if (const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsComplete_DelegateHandle);
	}
	
	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		OnFindSessionsComplete_Event.Broadcast(TArray<FOnlineSessionSearchResult>(), Success);
		return;
	}
	OnFindSessionsComplete_Event.Broadcast(LastSessionSearch->SearchResults,Success);
}

void UMultiplayerPortalSessionSubsystem::OnSessionsFound(const TArray<FOnlineSessionSearchResult>& SessionResults,
	bool Success)
{
	TArray<FindSearchResults> ExposedResultsArray;
	
	for (auto result: SessionResults)
	{
		ExposedResultsArray.Add(FindSearchResults(result.Session, result.PingInMs));
	}
	OnFindSessionsComplete_Exposed.Broadcast(ExposedResultsArray, Success);
}

void UMultiplayerPortalSessionSubsystem::OnJoinSessionCompleted(FName SessionName,
                                                                EOnJoinSessionCompleteResult::Type Result)
{
	if (const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionComplete_DelegateHandle);
	}
	
	OnJoinGameSessionComplete_Event.Broadcast(Result);
	TryTravelToCurrentSession();
}

void UMultiplayerPortalSessionSubsystem::OnSessionJoined(EOnJoinSessionCompleteResult::Type Result)
{
	OnJoinSessionsComplete_Exposed.Broadcast(FJoinSessionResult::Expose(Result));
}

bool UMultiplayerPortalSessionSubsystem::TryTravelToCurrentSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid()) return false;
	
	FString connectString;
	if (!sessionInterface->GetResolvedConnectString(NAME_GameSession, connectString)) return false;
	
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	// Set current session id from session settings map name
	sessionInterface->GetSessionSettings(NAME_GameSession)->Get(SETTING_MAPNAME, CurrentSessionID);
	playerController->ClientTravel(connectString, TRAVEL_Absolute);
	return true;
	
}
