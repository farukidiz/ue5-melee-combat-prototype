#pragma once

#include "CoreMinimal.h"
#include "CombatTargetingTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatTargetingState : uint8
{
	ECTS_Inactive UMETA(DisplayName = "Inactive"),
	ECTS_Searching UMETA(DisplayName = "Searching"),
	ECTS_TrackingPreferred UMETA(DisplayName = "Tracking Preferred"),
	ECTS_TrackingActive UMETA(DisplayName = "Tracking Active")
};

UENUM(BlueprintType)
enum class ECombatTargetValidity : uint8
{
	ECTV_Valid UMETA(DisplayName = "Valid"),
	ECTV_SoftInvalid UMETA(DisplayName = "Soft Invalid"),
	ECTV_HardInvalid UMETA(DisplayName = "Hard Invalid")
};


USTRUCT(BlueprintType)
struct FCombatTargetCandidate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float DistanceScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float FacingScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float StickyScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float ScreenCenterScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float CameraAlignmentScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float TotalScore = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bValid = false;
};

USTRUCT(BlueprintType)
struct FCombatTargetInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Target = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FVector TargetPoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector ToTarget = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector ToTarget2D = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsValid = false;
};