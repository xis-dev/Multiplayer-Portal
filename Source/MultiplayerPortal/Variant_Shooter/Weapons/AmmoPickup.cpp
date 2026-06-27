#include "AmmoPickup.h"

#include "MultiplayerPlayer.h"

AAmmoPickup::AAmmoPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RespawnTime = 5.0f;
}

void AAmmoPickup::PickupItem(AActor* PickingActor)
{
	if (auto* player = Cast<AMultiplayerPlayer>(PickingActor))
	{
		if (player->TryReloadWeapon_Server()) // Only pickup if the player could successfully reload
		{
			Super::PickupItem(PickingActor);
		}
	}
}

