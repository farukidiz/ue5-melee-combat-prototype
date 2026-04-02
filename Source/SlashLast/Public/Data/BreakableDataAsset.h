#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BreakableDataAsset.generated.h"

class UGeometryCollection;


UCLASS()
class SLASHLAST_API UBreakableDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UGeometryCollection* GeometryCollection;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector CapsuleLocation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float CapsuleHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float CapsuleRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpawnedTreasureHeightFromGround;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    USoundBase* BreakSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FDataTableRowHandle> AllowedTreasures;

	
};
