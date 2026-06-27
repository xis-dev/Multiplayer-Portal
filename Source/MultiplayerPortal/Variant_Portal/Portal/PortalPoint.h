#pragma once

#include "PortalPoint.generated.h"

// Testing points on the portal for placement and their offset in world-space from the center
USTRUCT(BlueprintType)
struct FPortalPoint
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector WorldPos{};
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Offset{};
};
