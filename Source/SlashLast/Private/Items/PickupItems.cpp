#include "Items/PickupItems.h"
#include "Kismet/KismetSystemLibrary.h"

void APickupItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const float LocationZ = GetActorLocation().Z;
	if (LocationZ > DesiredZ)
	{
		const FVector DeltaLocation = FVector{ 0.f, 0.f, DeltaTime * DriftRate };
		AddActorWorldOffset(DeltaLocation);
	}
}

void APickupItems::BeginPlay()
{

	Super::BeginPlay();
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, 2000.f);
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectType{ EObjectTypeQuery::ObjectTypeQuery1 };
	TArray<AActor*> ActorsToIgnore{ GetOwner() };
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ObjectType,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);

	DesiredZ = HitResult.ImpactPoint.Z + 125.f;

}
