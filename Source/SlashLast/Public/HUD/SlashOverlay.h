#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlashOverlay.generated.h"


class UProgressBar;
class UTextBlock;

UCLASS()
class SLASHLAST_API USlashOverlay : public UUserWidget
{
	GENERATED_BODY()


public:

	void SetHealthBarPercent(float Percent);
	void SetStaminaBarPercent(float Percent);

	void SetGold(int32 Gold);
	void SetSouls(int32 Souls);

private:

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaProgressBar;
	

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SoulText;
};
