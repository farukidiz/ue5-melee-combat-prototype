#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatTargetable.generated.h"

UINTERFACE(MinimalAPI)
class UCombatTargetable : public UInterface
{
	GENERATED_BODY()
};


class SLASHLAST_API ICombatTargetable
{
	GENERATED_BODY()

public:

	virtual bool CanBeTargeted() const = 0;
	virtual bool IsDeadForTargeting() const = 0;
	virtual FVector GetCombatTargetPoint() const = 0;
};
