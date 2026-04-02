#include "Items/Potion.h"
#include "Interfaces/PickupInterface.h"

void APotion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APotion::BeginPlay()
{
	Super::BeginPlay();

}

void APotion::SphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->HealCharacter(this);
		SpawnPickupEffect();
		SpawnPickupSound();

		Destroy();
	}
}
