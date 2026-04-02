#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TreasureDataTable.generated.h"


USTRUCT(BlueprintType)
struct FTreasureData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 GoldValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator ItemRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor RGBColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DropWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsEmpty;

};