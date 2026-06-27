#include "Pickup.h"

#include "Components/SphereComponent.h"


APickup::APickup()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	
	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the collision sphere
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SphereCollision->SetupAttachment(RootComponent);

	SphereCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 84.0f));
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldStatic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->bFillCollisionUnderneathForNavmesh = true;

	// subscribe to the collision overlap on the sphere
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnOverlap);

	// create the mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);

	Mesh->SetCollisionProfileName(FName("NoCollision"));
}


void APickup::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		// Server authorizes pickup
		PickupItem(OtherActor);
	}
}

void APickup::RespawnPickup()
{
	// unhide this pickup
	SetActorHiddenInGame(false);

	// call the BP handler
	BP_OnRespawn();
}

void APickup::PickupItem(AActor* PickingActor)
{
	// hide this mesh
	SetActorHiddenInGame(true);

	// disable collision
	SetActorEnableCollision(false);

	// disable ticking
	SetActorTickEnabled(false);

	// schedule the respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &APickup::RespawnPickup, RespawnTime, false);
}

void APickup::FinishRespawn()
{
	// enable collision
	SetActorEnableCollision(true);

	// enable tick
	SetActorTickEnabled(true);
}


