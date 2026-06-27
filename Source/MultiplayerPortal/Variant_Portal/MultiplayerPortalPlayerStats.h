#pragma once

#include "MultiplayerPortalPlayerStats.generated.h"

// Player stats in relation to gamemode, not including damage,health...
USTRUCT(BlueprintType, Blueprintable)
struct FMultiplayerPortalPlayerStats
{
	GENERATED_BODY()
	
	
	FLinearColor PlayerColor{FLinearColor::White};
	int32 KillCount{};
	int32 DeathCount{};
	
	
};
