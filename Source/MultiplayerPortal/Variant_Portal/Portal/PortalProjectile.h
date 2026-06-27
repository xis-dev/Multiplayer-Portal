// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"
#include "Teleportable.h"
#include "PortalProjectile.generated.h"

/**
 * Projectile that spawns a portal on impact with a wall
 */
UCLASS()
class MULTIPLAYERPORTAL_API APortalProjectile : public AShooterProjectile
{
	GENERATED_BODY()

public:
	
	APortalProjectile();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh{};
	
	
	// Set this when firing the projectile - true for orange, false for blue
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileColour, BlueprintReadWrite, Category = "Portal")
	bool bIsOrangePortal = true;
	
	
	UFUNCTION()
	void ChangeColour(bool bIsOrange);
	
	UFUNCTION()
	virtual void OnRep_ProjectileColour();
	
	
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<APortal> PortalClass;
	
	

protected:
	// Override NotifyHit to catch all blocking collisions
	virtual void NotifyHit(
		class UPrimitiveComponent* MyComp,
		AActor* Other,
		class UPrimitiveComponent* OtherComp,
		bool bSelfMoved,
		FVector HitLocation,
		FVector HitNormal,
		FVector NormalImpulse,
		const FHitResult& Hit) override;

private:
	bool CanPlacePortal(const FVector& Location, const FVector& Normal, AActor* HitActor, UPrimitiveComponent* HitComp) const;
	
};