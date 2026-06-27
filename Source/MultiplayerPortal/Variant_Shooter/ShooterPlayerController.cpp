// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "MultiplayerPortal.h"
#include "ShooterGameMode.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogMultiplayerPortal, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);
		} else {

			UE_LOG(LogMultiplayerPortal, Error, TEXT("Could not spawn bullet counter widget."));

		}
		
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}

void AShooterPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);

}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	OnBulletCountUpdated(0, 0);
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


void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}



void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}



