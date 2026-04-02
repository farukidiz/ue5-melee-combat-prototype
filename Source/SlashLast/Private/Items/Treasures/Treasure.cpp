#include "Items/Treasures/Treasure.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Data/TreasureDataTable.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"

ATreasure::ATreasure()
{
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATreasure::InitializeTreasure(const FTreasureData* Data)
{


	if (ItemMesh && Data && Data->Mesh && Data->Sound)
	{
		ItemMesh->SetStaticMesh(Data->Mesh);
		PickupSound = Data->Sound;
	}

	GoldValue = Data->GoldValue;
	this->SetActorRotation(Data->ItemRotation);
	FTimerHandle TreasureSpawnTimerHandle;
	GetWorldTimerManager().SetTimer(
		TreasureSpawnTimerHandle,
		this,
		&ATreasure::SetCollision,
		1.f,
		false
	);
	ItemEffect->SetVariableLinearColor(FName("User.Color"), Data->RGBColor);
}

void ATreasure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorWorldRotation(FRotator{ 0.f, DeltaTime * 45.f, 0.f });

}

void ATreasure::SphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->AddGold(this);
		SpawnPickupSound();
		Destroy();
	}



}
void ATreasure::SetCollision()
{
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

