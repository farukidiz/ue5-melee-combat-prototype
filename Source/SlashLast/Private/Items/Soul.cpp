#include "Items/Soul.h"
#include "Interfaces/PickupInterface.h"
#include "Kismet/KismetSystemLibrary.h"


void ASoul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASoul::BeginPlay()
{
	Super::BeginPlay();
}

void ASoul::SphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->AddSoul(this);

		SpawnPickupEffect();
		SpawnPickupSound();

		Destroy();
	}

}