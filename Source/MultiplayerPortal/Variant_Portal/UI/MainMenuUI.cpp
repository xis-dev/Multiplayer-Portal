// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuUI.h"

#include "OnlineSessionSettings.h"
#include "Algo/RandomShuffle.h"
#include "Online/OnlineSessionNames.h"
#include "Subsystems/MultiplayerPortalSessionSubsystem.h"


void UMainMenuUI::CreateSession()
{
	if (auto* localPlayer = GetOwningLocalPlayer())
	{
		// Only create sessions if machine is their own server and not the client of another(playing in listen server mode and trying to create session)
		if (!localPlayer->GetPlayerController(GetWorld())->HasAuthority()) return; 
		if (auto* subsystem = localPlayer->GetGameInstance()->GetSubsystem<UMultiplayerPortalSessionSubsystem>())
		{
			subsystem->OnCreateSessionComplete_Event.AddUniqueDynamic(this, &UMainMenuUI::OnSessionCreated);
			subsystem->CreateSession(4, true, GetRandomString(5));
		}
	}
}

void UMainMenuUI::TryJoinSession(const FString& id)
{
	if (auto* localPlayer = GetOwningLocalPlayer())
	{
		if (auto* subsystem = localPlayer->GetGameInstance()->GetSubsystem<UMultiplayerPortalSessionSubsystem>())
		{
			// Cache id we tried to join with to use while finding sessions
			CurrentJoiningID = id;
			subsystem->OnFindSessionsComplete_Exposed.AddUniqueDynamic(this, &UMainMenuUI::OnFoundSessions); // Bind to exposed findsession struct 
			subsystem->FindSessions(4, true);
		}
	}

}


void UMainMenuUI::OnSessionCreated(bool Success)
{
	if (Success)
	{
		// Take player to level as a listen server
		GetWorld()->ServerTravel(TEXT("/Game/Variant_Shooter/Lvl_Shooter?listen"));
	}
}

void UMainMenuUI::OnFoundSessions(TArray<FindSearchResults> SessionResults, bool Success)
{
	if (Success)
	{
		for (auto searchResult: SessionResults)
		{
			FString sessionMapName;
			searchResult.Session.SessionSettings.Get(SETTING_MAPNAME, sessionMapName);
			
			// Upon finding the last id we tried searching for, join the lobby
			if (CurrentJoiningID == sessionMapName)
			{
				if (auto* localPlayer = GetOwningLocalPlayer())
				{
					if (auto* subsystem = localPlayer->GetGameInstance()->GetSubsystem<UMultiplayerPortalSessionSubsystem>())
					{
						subsystem->OnJoinSessionsComplete_Exposed.AddUniqueDynamic(this, &UMainMenuUI::OnJoinedSession);
						subsystem->JoinGameSession(FOnlineSessionSearchResult(searchResult));
					}
				}
				break;
			}
			//IDsAndSessions.Add(sessionMapName, searchResult);
		}
	}
}

FString UMainMenuUI::GetRandomString(int32 length)
{
	FString allCharacters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	FString result{};
	for (int i = 0; i < length; ++i)
	{
		// Shuffle array and simply get the first element for randomization
		Algo::RandomShuffle(allCharacters);
		result += allCharacters[0];
	}
	return result;
}


void UMainMenuUI::OnJoinedSession_Implementation(FJoinSessionResult JoinResult)
{
	// Joined
}


