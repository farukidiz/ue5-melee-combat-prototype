#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarComponent.generated.h"

UCLASS()
class SLASHLAST_API UHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	void SetHealthPercentage(float Percent);
protected:

	void BeginPlay() override;

private:
	UPROPERTY()
	class UHealthBar* HealthBarWidgetObject;

};
