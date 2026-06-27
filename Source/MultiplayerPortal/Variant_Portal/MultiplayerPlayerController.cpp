// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerPlayerController.h"

#include "MultiplayerGameMode.h"
#include "MultiplayerGameState.h"
#include "MultiplayerPlayer.h"
#include "ShooterBulletCounterUI.h"
#include "ShooterGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/MultiplayerPortalSessionSubsystem.h"

class UMultiplayerPortalSessionSubsystem;

void AMultiplayerPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// Init kill & death count
	OnKillCountUpdated(0);
	OnDeathCountUpdated(0);
	if (auto* subsystem = GetGameInstance()->GetSubsystem<UMultiplayerPortalSessionSubsystem>())
	{
		UpdateSessionID(subsystem->CurrentSessionID);
	}
}

void AMultiplayerPlayerController::OnPossess(APawn* InPawn)
{
	// OnPosses runs on server only, get player info from gamemode & propagate to owning player
	Super::OnPossess(InPawn);
	if (auto* MultiplayerChar = Cast<AMultiplayerPlayer>(InPawn))
	{
		if (auto* GameMode = Cast<AMultiplayerGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			FMultiplayerPortalPlayerStats PlayerStats;
			if (GameMode->GetPlayerStats(this, PlayerStats))
			{
				MultiplayerChar->HandleNewStatValues(PlayerStats);
			}
		}
	}
	
	
}

void AMultiplayerPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);
	
	// subscribe to the pawn's OnDestroyed delegate
	P->OnDestroyed.AddDynamic(this, &AMultiplayerPlayerController::OnPawnDestroyed);
	
	if (AMultiplayerPlayer* MultiplayerPlayer = Cast<AMultiplayerPlayer>(P))
	{
		// add the player tag
		MultiplayerPlayer->Tags.Add(PlayerPawnTag);
	
		// Client subscribes and handles calling pawn delegates, any server sided information will be routed through rpc
		MultiplayerPlayer->OnBulletCountUpdated.AddDynamic(this, &AMultiplayerPlayerController::OnBulletCountUpdated);
		MultiplayerPlayer->OnDamaged.AddDynamic(this, &AMultiplayerPlayerController::OnPawnDamaged);
		MultiplayerPlayer->OnKillCountUpdated.AddDynamic(this, &AMultiplayerPlayerController::OnKillCountUpdated);
		MultiplayerPlayer->OnDeathCountUpdated.AddDynamic(this, &AMultiplayerPlayerController::OnDeathCountUpdated);
	
		// force update the life bar
		MultiplayerPlayer->OnDamaged.Broadcast(1.0f);
	}
	
	
	// Get local game state
	if (auto* gameState = Cast<AMultiplayerGameState>(UGameplayStatics::GetGameState(GetWorld())))
	{
		gameState->SetPortalLocalPlayer(P);
	}
	

	
	
}

void AMultiplayerPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (IsLocalController())
	{
		// Reset bullt ui
		OnBulletCountUpdated(0, 0);
		Server_OnPawnDestroyed();
	}
}

void AMultiplayerPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	if (IsLocalController())
	{
		if (BulletCounterUI)
		{
			BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
		}
	}
}

void AMultiplayerPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsLocalController())
	{
		if (IsValid(BulletCounterUI))
		{
			BulletCounterUI->BP_Damaged(LifePercent);
		}
	}
}

void AMultiplayerPlayerController::OnKillCountUpdated(int32 Kills)
{
	if (IsLocalController())
	{
		if (IsValid(BulletCounterUI))
		{
			BulletCounterUI->BP_UpdateKillCount(Kills);
		}
	}
}

void AMultiplayerPlayerController::OnDeathCountUpdated(int32 Deaths)
{
	if (IsLocalController())
	{
		if (IsValid(BulletCounterUI))
		{
			BulletCounterUI->BP_UpdateDeathCount(Deaths);
		}
	}
}

void AMultiplayerPlayerController::UpdateSessionID(FString SessionID)
{
	if (IsLocalController())
	{
		if (IsValid(BulletCounterUI))
		{
			BulletCounterUI->BP_UpdateSessionID(SessionID);
		}
	}
}

void AMultiplayerPlayerController::Server_OnPawnDestroyed_Implementation()
{
	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);
		
	if (ActorList.Num() > 0)
	{
		// select a random player start
		AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		// spawn a character at the player start
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();
			
		if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
		{
			// possess the character
			Possess(RespawnedCharacter);
		}
	}
}



