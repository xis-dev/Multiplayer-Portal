#pragma once

#include "CoreMinimal.h"
#include "MultiplayerPortalPlayerStats.h"
#include "ShooterCharacter.h"
#include "Portal/Teleportable.h"

#include "MultiplayerPlayer.generated.h"

class APortal;
class UBoxComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKillCountUpdated, int32, Kills);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathCountUpdated, int32, Deaths);

UCLASS(Blueprintable, BlueprintType)
class AMultiplayerPlayer: public AShooterCharacter, public ITeleportable
{
	GENERATED_BODY()
	
protected:
	
	// Input Actions
	
	/** Alt fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* AltFireAction;
	
	// Switch to previous weapon in owned weapons
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* PrevWeaponAction;
	
	// Switch to next weapon in owned weapons
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* NextWeaponAction;
	
	// Head Collider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UBoxComponent> HeadCollider;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UCapsuleComponent> BodyCollider;
	
	// Mesh Sockets
	
	UPROPERTY(EditAnywhere, Category = "Sockets")
	FName HeadSocket = FName("headSocket");
	
	UPROPERTY(EditAnywhere, Category = "Sockets")
	FName BodySocket = FName("BodySocket");
	
	// Player color
	UPROPERTY(ReplicatedUsing = OnRep_PlayerColor)
	FLinearColor PlayerColor = FLinearColor::White;
	
	// Kill Count
	UPROPERTY(ReplicatedUsing = OnRep_Kills)
	int32 Kills;
	
	// Death Count
	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
	
	// Timer Handles
	
	FTimerHandle WallCheckHandle;
	
	FTimerHandle TeleportHandle;
	
public:
	
	AMultiplayerPlayer();
	
	// In class definition, after UPROPERTY declarations
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Delegates
	
	FKillCountUpdated OnKillCountUpdated;
	
	FDeathCountUpdated OnDeathCountUpdated;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// Replicated variable callbacks

	UFUNCTION()
	void OnRep_PlayerColor();
	
	UFUNCTION()
	void OnRep_Kills();	
	
	UFUNCTION()
	void OnRep_Deaths();
	

	virtual void ReloadCurrentWeapon() override;
	
	UFUNCTION(Server, Reliable)
	void Server_ReloadWeapon();
	
	UFUNCTION()
	void SwitchToPreviousWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_SwitchToPreviousWeapon();
	
	UFUNCTION()
	void SwitchToNextWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_SwitchToNextWeapon();
	
	UFUNCTION()
	void CheckSpawnableWall();
	
	// Can see valid portal wall handling
	UFUNCTION(BlueprintNativeEvent)
	void CanSeeValidWall();
	
	// Cannot see valid portal wall handling
	UFUNCTION(BlueprintNativeEvent)
	void CannotSeeValidWall();
	
	// Callback to the client upon taking damage
	UFUNCTION(Client, Reliable)
	void Client_OnDamageTaken(float CurrentHealth, float MaxHealth); 

public:
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void DoStartFiring() override;
	
	virtual void DoStopFiring() override;
	
	// Server RPC for firing (to avoid client cheating)
	UFUNCTION(Server, Reliable)
	void ServerStartFiring();
	
	UFUNCTION(Server, Reliable)
	void ServerStopFiring();
	
	// Server RPC for alt firing (to avoid client cheating)
	UFUNCTION(Server, Reliable)
	void ServerStartAltFiring();
	
	UFUNCTION(Server, Reliable)
	void ServerStopAltFiring();
	
	/** Handles start alt firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartAltFiring();

	/** Handles stop alt firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStopAltFiring();
	
	virtual void DoSwitchWeapon() override;
	
	UFUNCTION(Server, Reliable)
	void ServerSwitchWeapon();

	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void HandleNewStatValues(FMultiplayerPortalPlayerStats stats);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stats")
	void Server_HandleNewStatValues(FMultiplayerPortalPlayerStats stats);
	
	// Boolean return function for successful reload, should only be called on server's machine
	UFUNCTION()
	bool TryReloadWeapon_Server();
	
	// Set player color (called by GameMode on spawn)
	void SetPlayerColor(FLinearColor NewColor);
	
	// Teleportable
	
	UPROPERTY()
	bool CanTeleport = true;

	virtual void HasBeenTeleported() override;
	
	virtual bool GetCanTeleport() override;
	
	virtual void ResetCanTeleport() override;

	UFUNCTION()
	void StartCheckingForPortalWalls();
	
	UFUNCTION()
	void StopCheckingForPortalWalls();

	
protected:
	
	virtual void Die() override;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();
	
	virtual void OnRespawn() override;
	
};
