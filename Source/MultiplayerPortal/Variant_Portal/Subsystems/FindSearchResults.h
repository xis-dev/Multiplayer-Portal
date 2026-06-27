#pragma once
#include "OnlineSessionSettings.h"

#include "FindSearchResults.generated.h"

// OnlineSessionSearchResult wrapper to expose to the class to ue replication
USTRUCT(Blueprintable, BlueprintType)
struct FindSearchResults
{
	GENERATED_BODY()
	
public:
	/** All advertised session information */
	FOnlineSession Session;
	/** Ping to the search result, MAX_QUERY_PING is unreachable */
	int32 PingInMs;

	FindSearchResults()
		: PingInMs(MAX_QUERY_PING)
	{
	}
	
	FindSearchResults(const FOnlineSession& _Session, int32 _Ping)
	: Session(_Session), PingInMs(_Ping)
	{
	}
	
	operator FOnlineSessionSearchResult() const
	{
		FOnlineSessionSearchResult searchResult;
		
		searchResult.Session = Session;
		searchResult.PingInMs = PingInMs;
		
		return searchResult;
	}

	/**
	 *	@return true if the search result is valid, false otherwise
	 */
	bool IsValid() const
	{
		return (Session.OwningUserId.IsValid() && IsSessionInfoValid());
	}

	/** 
	 * Check if the session info is valid, for cases where we don't need the OwningUserId
	 * @return true if the session info is valid, false otherwise
	 */
	bool IsSessionInfoValid() const
	{
		return (Session.SessionInfo.IsValid() && Session.SessionInfo->IsValid());
	}

	/** @return the session id for a given session search result */
	FString GetSessionIdStr() const
	{
		return Session.GetSessionIdStr();
	}
};
