#pragma once

#include "CoreMinimal.h"
#include "Items/PickupItems.h"
#include "Soul.generated.h"


UCLASS()
class SLASHLAST_API ASoul : public APickupItems
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;
	void SphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	int32 Soul;


public:

	FORCEINLINE void SetSoul(int32 NumberOfSouls) { Soul = NumberOfSouls; }
	FORCEINLINE int32 GetSoul() const { return Soul; }

};
