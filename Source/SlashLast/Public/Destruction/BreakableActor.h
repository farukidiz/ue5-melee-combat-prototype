#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"
#include "BreakableActor.generated.h"


class UFieldSystemMetaDataFilter;
class UGeometryCollectionComponent;
class UFieldSystemComponent;
class UBreakableDataAsset;
struct FChaosBreakEvent;

UCLASS()
class SLASHLAST_API ABreakableActor
    : public AActor, public IHitInterface
{
    GENERATED_BODY()

public:
    ABreakableActor();

    void OnConstruction(const FTransform& Transform) override;
    void BeginPlay() override;

    virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;

protected:

    UPROPERTY(VisibleAnywhere)
    UGeometryCollectionComponent* GeometryCollection;

    UPROPERTY(VisibleAnywhere)
    class UCapsuleComponent* CapsuleComponent;

    UPROPERTY(VisibleAnywhere)
    UFieldSystemComponent* FieldSystem;

    UPROPERTY(EditDefaultsOnly, Category = "Break")
    float BreakForceRadial = 1000000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Break")
    float BreakForceVector = 5000000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Break")
    float BreakRadius = 150.f;

private:

    void SpawnTreasure();

    void ApplyBreakField(const FVector& ImpactPoint);
    void PlayBreakSound(const FVector& ImpactPoint);

    UFUNCTION()
    void ChaosBreakEvent(const FChaosBreakEvent& BreakEvent);

    bool bBroken = false;

    UPROPERTY(EditAnywhere, Category = "Config")
    UBreakableDataAsset* BreakableConfig;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class ATreasure> TreasureClass;

};
