// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class AMultiplayerPlayer;
class AMainPlayer;
class UBoxComponent;
class USceneComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
UCLASS()
class APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();

	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UBoxComponent> BoxTrigger;

	// Scene capture that will be positioned relative to the other portal 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FinalPortalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RenderTargetMesh;
	
	// What does this portal display
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextureRenderTarget2D> DisplayRenderTarget;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PortalDynamicMaterial;
protected:


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
    UFUNCTION()
    void ActorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Portal")
	FTransform OtherPortalTransform;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Portal")
	APortal* OtherPortal;
	
	UPROPERTY()
	TObjectPtr<AMultiplayerPlayer> LocalPlayer;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void SetLocalPlayer(AMultiplayerPlayer* MPlayer){LocalPlayer = MPlayer;}
	
	// Returns normalized output direction of a given position relative to the other portal, if it exists
	UFUNCTION() 
	FVector GetOutputDirectionRelativeToOtherPortal(const FVector& Dir);
	
	APortal* GetOtherPortal() {return OtherPortal;};
	
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void SetPortalParams(APortal* other);

	// Change to multicast call on clients to change colour
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ColourChange(bool isOrange);
	
	UFUNCTION()
	void PortalSpawned(APortal* other, bool isOrange);
	
	static FRotator GetRotatorFromNormal(FVector impactNormal);
	
	
};
