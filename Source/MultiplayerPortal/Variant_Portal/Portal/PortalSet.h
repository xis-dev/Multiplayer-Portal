#pragma once

#include "PortalSet.generated.h"

class APortal;

// Simple struct to hold both connected portal types that may exist
USTRUCT(BlueprintType, Blueprintable)
struct FPortalSet
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<APortal> OrangePortal;
	
	UPROPERTY()
	TObjectPtr<APortal> BluePortal;
};
