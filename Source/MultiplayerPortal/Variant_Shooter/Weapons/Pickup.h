#pragma once

#include "Pickup.generated.h"


UCLASS(Blueprintable, BlueprintType)
class APickup: public AActor
{
	GENERATED_BODY()
	
protected:
	/** Collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereCollision;

	/** Pickup mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;
	
public:
	APickup();
	
	/** Called when it's time to respawn this pickup */
	virtual void RespawnPickup();
	
protected:
	/** Handles collision overlap */
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** Time to wait before respawning this pickup */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 0, ClampMax = 120, Units = "s"))
	float RespawnTime = 5.0f;

	/** Timer to respawn the pickup */
	FTimerHandle RespawnTimer;
	
	
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual void PickupItem(AActor* PickingActor);

	/** Passes control to Blueprint to animate the pickup respawn. Should end by calling FinishRespawn */
	UFUNCTION(BlueprintImplementableEvent, Category="Pickup", meta = (DisplayName = "OnRespawn"))
	void BP_OnRespawn();

	/** Enables this pickup after respawning */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void FinishRespawn();
};
