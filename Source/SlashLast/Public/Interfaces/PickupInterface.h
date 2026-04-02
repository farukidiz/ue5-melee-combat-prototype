#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"


UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

class SLASHLAST_API IPickupInterface
{
	GENERATED_BODY()

public:

	virtual void SetOverlappingItem(class AItems* Item) = 0;
	virtual void AddSoul(class ASoul* Soul) = 0;
	virtual void AddGold(class ATreasure* Gold) = 0;
	virtual void HealCharacter(class APotion* Potion) = 0;
};
