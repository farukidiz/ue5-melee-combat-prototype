#include "HUD/HealthBarComponent.h"
#include "HUD/HealthBar.h"
#include "Components/ProgressBar.h"

void UHealthBarComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UUserWidget* WidgetObject = GetUserWidgetObject())
	{
		HealthBarWidgetObject = Cast<UHealthBar>(WidgetObject);
	}
}

void UHealthBarComponent::SetHealthPercentage(float Percent)
{
	if (HealthBarWidgetObject && HealthBarWidgetObject->HealthBar)
	{
		HealthBarWidgetObject->HealthBar->SetPercent(Percent);
	}
}