#pragma once

#include "Interfaces/OnlineSessionInterface.h"
#include "JoinSessionResult.generated.h"

UENUM(BlueprintType)
enum EJoinSessionResultExposed
{
		Success,
		/** There are no open slots to join */
		SessionIsFull,
		/** The session couldn't be found on the service */
		SessionDoesNotExist,
		/** There was an error getting the session server's address */
		CouldNotRetrieveAddress,
		/** The user attempting to join is already a member of the session */
		AlreadyInSession,
		/** An error not covered above occurred */
		UnknownError
};

// OnJoinSettingComplete result wrapper to expose the enum to ue replication
USTRUCT(BlueprintType)
struct FJoinSessionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EJoinSessionResultExposed> Type;

	static FJoinSessionResult Expose(EOnJoinSessionCompleteResult::Type JoinResult)
	{
		FJoinSessionResult Result;

		switch (JoinResult)
		{
		case EOnJoinSessionCompleteResult::Success:
			Result.Type = Success;
			break;
		case EOnJoinSessionCompleteResult::SessionIsFull:
			Result.Type = SessionIsFull;
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			Result.Type = SessionDoesNotExist;
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			Result.Type = CouldNotRetrieveAddress;
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			Result.Type = AlreadyInSession;
			break;
		default:
			Result.Type = UnknownError;
			break;
		}

		return Result;
	}
};
