// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "ShooterWeapon.generated.h"

class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

/**
 *  Base class for a simple first person shooter weapon
 *  Provides both first person and third person perspective meshes
 *  Handles ammo and firing logic
 *  Interacts with the weapon owner through the ShooterWeaponHolder interface
 */
UCLASS(abstract)
class MULTIPLAYERPORTAL_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
	/** First person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** Third person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;

protected:

	// After the UPROPERTY declarations, add:

	


/** Current weapon owner's controller (cached for damage) */
UPROPERTY()
TObjectPtr<AController> WeaponOwnerController;
	UPROPERTY(EditAnywhere, Category = "Firing")
	bool bUseLineTrace = false;

	// can be removed I think, but i'm eepy
	UPROPERTY(EditAnywhere, Category = "Firing")
	float LineTraceDamage = 20.0f;


	UPROPERTY(EditAnywhere, Category = "Firing", meta = (ClampMin = 0, Units = "cm"))
	float LineTraceRange = 10000.0f;

	// Radial damage parameters (for grenade)
	UPROPERTY(EditAnywhere, Category = "Firing")
	float RadialDamageRadius = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Firing")
	float RadialDamageFalloff = 1.0f;

	// PISTOL ONLY
	UPROPERTY(EditAnywhere, Category = "Firing")
	float HeadshotMultiplier = 100.0f;  // instant kill
	
	
	
	/** Cast pointer to the weapon owner */
	IShooterWeaponHolder* WeaponOwner;

	/** Type of projectiles this weapon will shoot */
	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	/** Number of bullets in a magazine */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 100))
	int32 MagazineSize = 10;


	
	/** Animation montage to play when firing this weapon */
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage;

	/** AnimInstance class to set for the first person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** AnimInstance class to set for the third person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	/** Cone half-angle for variance while aiming */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float AimVariance = 0.0f;

	/** Amount of firing recoil to apply to the owner */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 100))
	float FiringRecoil = 0.0f;

	/** Name of the first person muzzle socket where projectiles will spawn */
	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName;

	/** Distance ahead of the muzzle that bullets will spawn at */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	/** If true, this weapon will automatically fire at the refire rate */
	UPROPERTY(EditAnywhere, Category="Refire")
	bool bFullAuto = false;

	// If this weapon has an alternate fire
	UPROPERTY(EditAnywhere, Category = "Alt Fire")
	bool bCanAltFire = false;
	
	/** Time between shots for this weapon. Affects both full auto and semi auto modes */
	UPROPERTY(EditAnywhere, Category="Refire", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float RefireRate = 0.5f;
	
	/** Time between shots for this weapon. Affects both full auto and semi auto modes */
	UPROPERTY(EditAnywhere, Category="Refire", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float AltFireRate = 0.5f;
	

	/** Game time of last shot fired, used to enforce refire rate on semi auto */
	float TimeOfLastShot = 0.0f;
	
	// Game time of last alt fire shot fired
	float TimeOfLastAltFire = 0.0f;

	/** If true, the weapon is currently firing */
	bool bIsFiring = false;
	
	// Is the weapon currently alt firing
	bool bIsAltFiring;

	/** Timer to handle full auto refiring */
	FTimerHandle RefireTimer;
	
	// Timer to handle full auto alt firing
	FTimerHandle AltRefireTimer;

	/** Cast pawn pointer to the owner for AI perception system interactions */
	TObjectPtr<APawn> PawnOwner;

	/** Loudness of the shot for AI perception system interactions */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100))
	float ShotLoudness = 1.0f;

	/** Max range of shot AI perception noise */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float ShotNoiseRange = 3000.0f;

	/** Tag to apply to noise generated by shooting this weapon */
	UPROPERTY(EditAnywhere, Category="Perception")
	FName ShotNoiseTag = FName("Shot");

	UPROPERTY(ReplicatedUsing = OnRep_WeaponActive)
	bool WeaponActive;

	/** Number of bullets in the current magazine */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentBullets)
	int32 CurrentBullets = 0;
	
public:	

	/** Constructor */
	AShooterWeapon();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	

protected:
	
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay Cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	/** Called when the weapon's owner is destroyed */
	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

public:

	/** Activates this weapon and gets it ready to fire */
	UFUNCTION(Server, Reliable)
	void ActivateWeapon();
	
	
	UFUNCTION()
	void OnRep_WeaponActive();
	
	UFUNCTION()
	virtual void OnRep_CurrentBullets();
	

	/** Deactivates this weapon */
	UFUNCTION(Server, Reliable)
	void DeactivateWeapon();
	
	/** Server RPC to perform the actual shot */
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerFire();
	
	// Callback to client once fired, may be used for recoil, ui....
	UFUNCTION(Client, Reliable)
	virtual void ClientOnFired();
	
	/** The real firing logic (called on server) */
	virtual void ExecuteFire();

	/** Start firing this weapon */
	void StartFiring();

	/** Stop firing this weapon */
	void StopFiring();
	
	/** Server RPC to perform the actual shot */
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerAltFire();
	
	/** The real firing logic (called on server) */
	virtual void ExecuteAltFire();
	
	// Start alt firing this weapon
	virtual void StartAltFiring();
	
	// Stop alt firing this weapon
	virtual void StopAltFiring();
	
	void AddBullets(int32 Amount);

protected:

	/** Fire the weapon */
	virtual void Fire();
	
	// Alternate Fire the weapon
	virtual void AltFire();

	/** Called when the refire rate time has passed while shooting semi auto weapons */
	void FireCooldownExpired();
	

	/** Fire a projectile towards the target location */
	virtual AShooterProjectile* FireProjectile(const FVector& TargetLocation);

	/** Calculates the spawn transform for projectiles shot by this weapon */
	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;
	
	void PerformLineTrace();
	
	// Try to teleport line trace through portal
	bool TeleportLineTraceThroughPortal(const FHitResult& Hit, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params);

public:

	/** Returns the first person mesh */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; };

	/** Returns the third person mesh */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; };

	/** Returns the first person anim instance class */
	const TSubclassOf<UAnimInstance>& GetFirstPersonAnimInstanceClass() const;

	/** Returns the third person anim instance class */
	const TSubclassOf<UAnimInstance>& GetThirdPersonAnimInstanceClass() const;

	/** Returns the magazine size */
	int32 GetMagazineSize() const { return MagazineSize; };

	/** Returns the current bullet count */
	int32 GetBulletCount() const { return CurrentBullets; }
};
