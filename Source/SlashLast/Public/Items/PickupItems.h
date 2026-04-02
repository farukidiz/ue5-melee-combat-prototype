#pragma once

#include "CoreMinimal.h"
#include "Items/Items.h"
#include "PickupItems.generated.h"

UCLASS()
class SLASHLAST_API APickupItems : public AItems
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

private:
	double DesiredZ;

	UPROPERTY(EditAnywhere, Category = "Pickup Properties")
	float DriftRate = -15.f;
};
