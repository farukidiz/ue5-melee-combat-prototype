#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatAttackSetDataAsset.generated.h"

UENUM(BlueprintType)
enum class ECombatAttackCategory : uint8
{
	ECAC_None,
	ECAC_CloseCombo,
	ECAC_GapCloser,
	ECAC_Heavy,
	ECAC_Finisher
};

USTRUCT(BlueprintType)
struct FCombatAttackDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttackID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECombatAttackCategory Category = ECombatAttackCategory::ECAC_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttackSection = FName("Attack");

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ExitSection = FName("Exit");

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ComboIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseMotionWarp = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanChainToNext = false;
};

UCLASS(BlueprintType)
class SLASHLAST_API UCombatAttackSetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCombatAttackDefinition> Attacks;
};