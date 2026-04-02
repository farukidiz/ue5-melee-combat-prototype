#pragma once

#include "CoreMinimal.h"
#include "Items/Items.h"
#include "Treasure.generated.h"

struct FTreasureData;

UCLASS()
class SLASHLAST_API ATreasure : public AItems
{
	GENERATED_BODY()

public:

	ATreasure();
	void InitializeTreasure(const FTreasureData* Data);
	void Tick(float DeltaTime) override;

	FORCEINLINE int32 GetGold() const { return GoldValue; }

protected:

	void SphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	void SetCollision();

	UPROPERTY()
	int32 GoldValue;

};


