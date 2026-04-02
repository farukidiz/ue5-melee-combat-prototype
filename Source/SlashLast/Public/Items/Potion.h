#pragma once

#include "CoreMinimal.h"
#include "Items/PickupItems.h"
#include "Potion.generated.h"


UCLASS()
class SLASHLAST_API APotion : public APickupItems
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;
protected:

	virtual void BeginPlay() override;
	void SphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

	UPROPERTY(EditAnywhere, Category = "Potion Properties")
	float PotionHealth = 10.f;

public:
	float GetPotionHealth() const { return PotionHealth; }
	
};
