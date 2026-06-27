#pragma once

#include "Pickup.h"

#include "AmmoPickup.generated.h"

UCLASS (Blueprintable, BlueprintType)
class AAmmoPickup: public APickup
{
	GENERATED_BODY()
public:
	
	AAmmoPickup();
	
protected:
	// Pickup
	virtual void PickupItem(AActor* PickingActor) override;
};
