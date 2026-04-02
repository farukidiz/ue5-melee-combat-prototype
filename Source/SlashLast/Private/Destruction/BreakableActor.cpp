#include "Destruction/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Items/Treasures/Treasure.h"
#include "Components/CapsuleComponent.h"
#include "Data/TreasureDataTable.h"
#include "Data/BreakableDataAsset.h"

ABreakableActor::ABreakableActor()
{
    PrimaryActorTick.bCanEverTick = false;

    GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
    SetRootComponent(GeometryCollection);
    GeometryCollection->SetGenerateOverlapEvents(true);
    GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetupAttachment(GetRootComponent());
    CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

    FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
    FieldSystem->SetupAttachment(RootComponent);


}

void ABreakableActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (!BreakableConfig) return;

    if(BreakableConfig->GeometryCollection)
        GeometryCollection->SetRestCollection(BreakableConfig->GeometryCollection);

    if (CapsuleComponent)
    {
        CapsuleComponent->SetRelativeLocation(BreakableConfig->CapsuleLocation);
        CapsuleComponent->SetCapsuleHalfHeight(BreakableConfig->CapsuleHeight);
        CapsuleComponent->SetCapsuleRadius(BreakableConfig->CapsuleRadius);
    }

}

void ABreakableActor::BeginPlay()
{
    Super::BeginPlay();

    GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::ChaosBreakEvent);
}

void ABreakableActor::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{
    if (bBroken) return;

    bBroken = true;
    UE_LOG(LogTemp, Warning, TEXT("BreakableActor Hit!"));
    ApplyBreakField(ImpactPoint);
    CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);;
    PlayBreakSound(ImpactPoint);
    SpawnTreasure();
}


void ABreakableActor::ApplyBreakField(const FVector& ImpactPoint)
{
    if (!FieldSystem) return;


    URadialFalloff* RadialFalloff = NewObject<URadialFalloff>();

    RadialFalloff->SetRadialFalloff(
        BreakForceRadial,        // Magnitude
        0.0f,               // Min Range
        1.f,               // Max Range
        0.f,               // Default
        BreakRadius,       // Radius
        ImpactPoint,       // Position
        EFieldFalloffType::Field_Falloff_Linear
    );


    URadialVector* RadialVector = NewObject<URadialVector>();

    RadialVector->SetRadialVector(
        BreakForceVector,            // Force magnitude
        ImpactPoint
    );



    UFieldSystemMetaDataFilter* MetaData =
        NewObject<UFieldSystemMetaDataFilter>();

    MetaData->SetMetaDataFilterType(
        EFieldFilterType::Field_Filter_Dynamic,
        EFieldObjectType::Field_Object_Destruction,
        EFieldPositionType::Field_Position_CenterOfMass
    );


    FieldSystem->ApplyPhysicsField(
        true,
        EFieldPhysicsType::Field_ExternalClusterStrain,
        nullptr,
        RadialFalloff
    );


    FieldSystem->ApplyPhysicsField(
        true,
        EFieldPhysicsType::Field_LinearForce,
        MetaData,
        RadialVector
    );

}


void ABreakableActor::SpawnTreasure()
{
    if (!TreasureClass || !BreakableConfig || BreakableConfig->AllowedTreasures.Num() == 0)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;


    float TotalWeight = 0.f;
    for (const FDataTableRowHandle& RowHandle : BreakableConfig->AllowedTreasures)
    {
        if (!RowHandle.DataTable) continue;

        FTreasureData* Row = RowHandle.DataTable->FindRow<FTreasureData>(RowHandle.RowName, TEXT("CalculateWeight"));
        if (Row)
            TotalWeight += Row->DropWeight;
    }

    if (TotalWeight <= 0.f)
        return;


    float RandomValue = FMath::FRandRange(0.f, TotalWeight);
    float RunningSum = 0.f;
    FTreasureData* SelectedRow = nullptr;

    for (const FDataTableRowHandle& RowHandle : BreakableConfig->AllowedTreasures)
    {
        if (!RowHandle.DataTable) continue;

        FTreasureData* Row = RowHandle.DataTable->FindRow<FTreasureData>(RowHandle.RowName, TEXT("SelectWeighted"));
        if (!Row) continue;

        RunningSum += Row->DropWeight;
        if (RandomValue <= RunningSum)
        {
            SelectedRow = Row;
            break;
        }
    }

    if (!SelectedRow || SelectedRow->bIsEmpty)
        return;


    FVector SpawnLocation = GetActorLocation();
    SpawnLocation.Z += BreakableConfig->SpawnedTreasureHeightFromGround;  

    ATreasure* SpawnedTreasure = World->SpawnActor<ATreasure>(TreasureClass, SpawnLocation, GetActorRotation());
    if (SpawnedTreasure)
    {
        SpawnedTreasure->InitializeTreasure(SelectedRow);
    }
}


void ABreakableActor::ChaosBreakEvent(const FChaosBreakEvent& BreakEvent)
{
    this->SetLifeSpan(3.f);
}

void ABreakableActor::PlayBreakSound(const FVector& ImpactPoint)
{
    if (BreakableConfig && BreakableConfig->BreakSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BreakableConfig->BreakSound, ImpactPoint);
    }
}