#include "Components/CombatTargetingComponent.h"
#include "Data/CombatTargetingTypes.h"
#include "Interfaces/CombatTargetable.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"


UCombatTargetingComponent::UCombatTargetingComponent() : TargetingState(ECombatTargetingState::ECTS_Inactive)
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	StartScanTimer();
}

void UCombatTargetingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopScanTimer();

	Super::EndPlay(EndPlayReason);
}

void UCombatTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatTargetingComponent::UpdateTargeting()
{

	TargetingState = ECombatTargetingState::ECTS_Searching;

	ScanForCandidates();
	EvaluatePreferredTarget();
	UpdateActiveCombatTarget();

	if (ActiveCombatTarget.IsValid())
	{
		TargetingState = ECombatTargetingState::ECTS_TrackingActive;
	}
	else if (PreferredTarget.IsValid())
	{
		TargetingState = ECombatTargetingState::ECTS_TrackingPreferred;
	}
	else
	{
		TargetingState = ECombatTargetingState::ECTS_Inactive;
	}

	if (bEnableDebugDraw)
	{
		DebugDraw();
	}

	if (bEnableDebugLogs)
	{
		DebugLogSelection();
	}
}

void UCombatTargetingComponent::ScanForCandidates()
{
	CachedCandidates.Reset();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;


	TArray<AActor*> FoundActors;
	if (!SearchActors(FoundActors)) return;

	for (AActor* Actor : FoundActors)
	{
		const bool bCachedCondition = IsValid(Actor) && Actor != OwnerActor;
		FCombatTargetCandidate Candidate;
		if (bCachedCondition && TryBuildCandidate(Actor, Candidate))
		{
			CachedCandidates.Add(Candidate);
		}
	}
}

bool UCombatTargetingComponent::SearchActors(TArray<AActor*>& OutActors) const
{
	OutActors.Reset();

	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();

	if (!OwnerActor || !World)
	{
		return false;
	}

	static const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	TArray<AActor*> ActorsToIgnore = { OwnerActor };

	return UKismetSystemLibrary::SphereOverlapActors(
		World,
		OwnerActor->GetActorLocation(),
		ScanRadius,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OutActors
	);
}


void UCombatTargetingComponent::EvaluatePreferredTarget()
{
	
	SetPreferredTarget(SelectPreferredTarget());
}

void UCombatTargetingComponent::UpdateActiveCombatTarget()
{
	AActor* CurrentActive = ActiveCombatTarget.Get();
	AActor* CurrentPreferred = PreferredTarget.Get();

	if (!IsValid(CurrentActive))
	{
		SwitchActiveCombatTarget(CurrentPreferred,
							TEXT("NoActiveOrInvalidPointer"));
		return;
	}

	const ECombatTargetValidity Validity = EvaluateActiveTargetValidity(CurrentActive);
	if (bAttackCommitWindowActive)
	{
		HandleCommitWindowActiveTarget(Validity);
	}
	else
	{

		switch (Validity)
		{
		case ECombatTargetValidity::ECTV_Valid:
			HandleValidActiveTarget(CurrentActive, CurrentPreferred);
			return;

		case ECombatTargetValidity::ECTV_HardInvalid:
			HandleHardInvalidActiveTarget(CurrentPreferred);
			return;

		case ECombatTargetValidity::ECTV_SoftInvalid:
			HandleSoftInvalidActiveTarget(CurrentActive, CurrentPreferred);
			return;
		}

		SwitchActiveCombatTarget(CurrentPreferred,
			TEXT("UnexpectedValidityFallback"));

	}

}

void UCombatTargetingComponent::HandleValidActiveTarget(AActor* CurrentActive, AActor* CurrentPreferred)
{
	ResetActiveTargetGraceState();

	const float ActiveScore = GetCandidateScore(CurrentActive);
	ActiveCombatTargetLastKnownScore = ActiveScore;

	if (!IsValid(CurrentPreferred) || CurrentPreferred == CurrentActive)
	{
		return;
	}

	const float PreferredScore = GetCandidateScore(CurrentPreferred);
	const bool bShouldUpgrade = ActiveScore >= 0.f &&
								PreferredScore >= 0.f &&
								PreferredScore > (ActiveScore + ActiveValidSwitchScoreThreshold);

	if (bShouldUpgrade)
	{
		SwitchActiveCombatTarget(CurrentPreferred, TEXT("ValidStateScoreUpgrade"));
	}
}

void UCombatTargetingComponent::HandleHardInvalidActiveTarget(AActor* CurrentPreferred)
{
	SwitchActiveCombatTarget(CurrentPreferred, 
						TEXT("HardInvalid"));
}

void UCombatTargetingComponent::HandleSoftInvalidActiveTarget(AActor* CurrentActive, AActor* CurrentPreferred)
{
	if (IsValid(CurrentPreferred))
	{
		const float PreferredScore = GetCandidateScore(CurrentPreferred);
		const float ActiveReferenceScore = ActiveCombatTargetLastKnownScore;

		const bool bSwitchCondition = PreferredScore >= 0.f &&
									ActiveReferenceScore >= 0.f &&
									PreferredScore > (ActiveReferenceScore + ActiveSoftSwitchScoreThreshold);

		if (bSwitchCondition)
		{
			SwitchActiveCombatTarget(CurrentPreferred, TEXT("SoftInvalidScoreBasedHandoff"));
			return;
		}
	}

	bActiveTargetInGracePeriod = true;
	ActiveTargetInvalidElapsed += ScanInterval;

	UE_LOG(LogTemp, Warning,
		TEXT("ActiveCombatTarget=%s | SoftInvalid | Preferred=%s | ActiveScore=%.2f | PreferredScore=%.2f | Grace running: %.2f / %.2f"),
		*GetNameSafe(CurrentActive),
		*GetNameSafe(CurrentPreferred),
		ActiveCombatTargetLastKnownScore,
		GetCandidateScore(CurrentPreferred),
		ActiveTargetInvalidElapsed,
		ActiveTargetGracePeriod);

	if (ActiveTargetInvalidElapsed < ActiveTargetGracePeriod)
	{
		return;
	}

	const ECombatTargetValidity RecheckedValidity = EvaluateActiveTargetValidity(CurrentActive);

	if (RecheckedValidity == ECombatTargetValidity::ECTV_Valid)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Grace expired but old ActiveCombatTarget became valid again: %s"),
			*GetNameSafe(CurrentActive));

		ResetActiveTargetGraceState();
		return;
	}

	SwitchActiveCombatTarget(CurrentPreferred, 
						TEXT("GraceExpiredStillInvalid"));

}

void UCombatTargetingComponent::HandleCommitWindowActiveTarget(ECombatTargetValidity Validity)
{
	if (Validity == ECombatTargetValidity::ECTV_HardInvalid)
	{
		HandleHardInvalidActiveTarget(PreferredTarget.Get());
		return;
	}

	ResetActiveTargetGraceState();
}

ECombatTargetValidity UCombatTargetingComponent::EvaluateActiveTargetValidity(AActor* Actor) const
{
	bool bHasValidActorAndInterface = IsValid(Actor) && Actor->GetClass()->ImplementsInterface(UCombatTargetable::StaticClass());
	if (!bHasValidActorAndInterface)
	{
		return ECombatTargetValidity::ECTV_HardInvalid;
	}

	const ICombatTargetable* Targetable = Cast<ICombatTargetable>(Actor);
	bool bIsHardTargetable = Targetable && !Targetable->IsDeadForTargeting();
	if (!bIsHardTargetable)
	{
		return ECombatTargetValidity::ECTV_HardInvalid;
	}


	if (FindCandidateForActor(Actor) == nullptr)
	{
		return ECombatTargetValidity::ECTV_SoftInvalid;
	}

	return ECombatTargetValidity::ECTV_Valid;
}

void UCombatTargetingComponent::SwitchActiveCombatTarget(AActor* NewTarget, const TCHAR* Reason)
{
	UE_LOG(LogTemp, Warning,
		TEXT("SwitchActiveCombatTarget | Reason=%s | NewTarget=%s"),
		Reason,
		*GetNameSafe(NewTarget));

	ResetActiveTargetGraceState();
	SetActiveCombatTarget(NewTarget);
}

const FCombatTargetCandidate* UCombatTargetingComponent::FindCandidateForActor(AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	for (const FCombatTargetCandidate& Candidate : CachedCandidates)
	{
		if (Candidate.bValid && Candidate.Actor.Get() == Actor)
		{
			return &Candidate;
		}
	}

	return nullptr;
}

float UCombatTargetingComponent::GetCandidateScore(AActor* Actor) const
{
	const FCombatTargetCandidate* Candidate = FindCandidateForActor(Actor);
	return Candidate ? Candidate->TotalScore : -1.f;
}

void UCombatTargetingComponent::SetPreferredTarget(AActor* NewTarget)
{
	AActor* OldTarget = PreferredTarget.Get();

	if (OldTarget == NewTarget) return;

	PreferredTarget = NewTarget;
	OnPreferredTargetChanged.Broadcast(OldTarget, NewTarget);
}

void UCombatTargetingComponent::SetActiveCombatTarget(AActor* NewTarget)
{
	AActor* OldTarget = ActiveCombatTarget.Get();

	if (OldTarget == NewTarget)
	{
		return;
	}

	ActiveCombatTarget = NewTarget;
	OnActiveCombatTargetChanged.Broadcast(OldTarget, NewTarget);
}

bool UCombatTargetingComponent::TryBuildCandidate(AActor* Actor, FCombatTargetCandidate& OutCandidate) const
{
	if (!IsActorTargetable(Actor)) return false;

	const ICombatTargetable* Targetable = Cast<ICombatTargetable>(Actor);
	if (!Targetable) return false;

	// Geometry data
	const FVector OwnerLocation = GetOwnerLocation();
	const FVector OwnerForward = GetOwnerForwardVector();
	const FVector TargetPoint = Targetable->GetCombatTargetPoint();

	FVector ToTarget = TargetPoint - OwnerLocation;
	const float Distance = ToTarget.Size2D();

	if (Distance > ScanRadius) return false;

	ToTarget.Z = 0.f;
	const FVector ToTargetDir = ToTarget.GetSafeNormal();
	const float FacingDot = CalculateFacingDot(OwnerForward, ToTargetDir);

	const float CameraDot = CalculateCameraDot(ToTargetDir);

	const bool bPassFacingGate = FacingDot >= MinFacingDot;
	const bool bPassCameraGate = CameraDot >= MinCameraDot;

	UE_LOG(LogTemp, Warning,
		TEXT("CandidateCheck %s | FacingDot=%.2f | CameraDot=%.2f | PassFacing=%d | PassCamera=%d"),
		*GetNameSafe(Actor),
		FacingDot,
		CameraDot,
		bPassFacingGate,
		bPassCameraGate);

	if (!bPassFacingGate && !bPassCameraGate)
	{
		return false;
	}

	// Scores
	const float DistanceScore = CalculateDistanceScore(Distance);
	const float FacingScore = CalculateFacingScore(FacingDot);
	const float CameraAlignmentScore = CalculateCameraAlignmentScore(CameraDot);
	const float ScreenCenterScore = bUseScreenCenterScore ? CalculateScreenCenterScore(TargetPoint) : 0.f;

	const float TotalScore = DistanceScore * DistanceWeight +
		FacingScore * FacingWeight +
		CameraAlignmentScore * CameraAlignmentWeight +
		ScreenCenterScore * ScreenCenterWeight;



	// Output
	OutCandidate.Actor = Actor;
	OutCandidate.Distance = Distance;
	OutCandidate.DistanceScore = DistanceScore;
	OutCandidate.FacingScore = FacingScore;
	OutCandidate.CameraAlignmentScore = CameraAlignmentScore;
	OutCandidate.ScreenCenterScore = ScreenCenterScore;
	OutCandidate.TotalScore = TotalScore;
	OutCandidate.bValid = true;

	return true;
}

FVector UCombatTargetingComponent::GetOwnerLocation() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
}

FVector UCombatTargetingComponent::GetOwnerForwardVector() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FVector::ZeroVector;
	}

	FVector Forward = OwnerActor->GetActorForwardVector();

	return FVector{ Forward.X, Forward.Y, 0.f }.GetSafeNormal();
}

FVector UCombatTargetingComponent::GetCameraForwardVector() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return FVector::ZeroVector;
	}

	const AController* Controller = OwnerPawn->GetController();
	if (!Controller)
	{
		return FVector::ZeroVector;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	FVector Forward = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);
	Forward.Z = 0.f;

	return Forward.GetSafeNormal();
}

bool UCombatTargetingComponent::IsActorTargetable(AActor* Actor) const
{
	const bool bActorCondition = IsValid(Actor) && 
								Actor->GetClass()->ImplementsInterface(UCombatTargetable::StaticClass());

	if (!bActorCondition) return false;


	const ICombatTargetable* Targetable = Cast<ICombatTargetable>(Actor);
	const bool bTargatebleCondition = Targetable && 
									Targetable->CanBeTargeted() && 
									!Targetable->IsDeadForTargeting();

	if (!bTargatebleCondition) return false;


	return true;
}

float UCombatTargetingComponent::CalculateDistanceScore(float Distance) const
{
	return 1.f - FMath::Clamp(Distance / ScanRadius, 0.f, 1.f);
}

float UCombatTargetingComponent::CalculateFacingScore(float FacingDot) const
{
	return (FacingDot + 1.f) * 0.5f;
}

float UCombatTargetingComponent::CalculateScreenCenterScore(const FVector& WorldLocation) const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return 0.f;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return 0.f;


	FVector2D ScreenPosition;
	const bool bProjected = PlayerController->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition);
	if (!bProjected)
	{
		return 0.f;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	if (ViewportX <= 0 || ViewportY <= 0)
	{
		return 0.f;
	}


	const FVector2D ScreenFocus(ViewportX * 0.5f, ViewportY * 0.25f);
	const float DistanceToFocus = FVector2D::Distance(ScreenPosition, ScreenFocus);

	const float MaxX = FMath::Max(ScreenFocus.X, ViewportX - ScreenFocus.X);
	const float MaxY = FMath::Max(ScreenFocus.Y, ViewportY - ScreenFocus.Y);
	const float MaxDistance = FVector2D(MaxX, MaxY).Size();

	const float NormalizedDistance = FMath::Clamp(DistanceToFocus / MaxDistance, 0.f, 1.f);
	return 1.f - NormalizedDistance;
}

float UCombatTargetingComponent::CalculateCameraAlignmentScore(float CameraDot) const
{
	return (CameraDot + 1.f) * 0.5f;
}

float UCombatTargetingComponent::CalculateFacingDot(const FVector& OwnerForward, const FVector& ToTargetDir) const
{
	return FVector::DotProduct(OwnerForward, ToTargetDir);
}

float UCombatTargetingComponent::CalculateCameraDot(const FVector& ToTargetDir) const
{
	const FVector CameraForward = GetCameraForwardVector();
	if (CameraForward.IsNearlyZero() || ToTargetDir.IsNearlyZero())
	{
		return -1.f;
	}

	return FVector::DotProduct(CameraForward, ToTargetDir);
}

AActor* UCombatTargetingComponent::SelectPreferredTarget() const
{
	AActor* BestTarget = nullptr;
	float BestScore = -1.f;

	for (const FCombatTargetCandidate& Candidate : CachedCandidates)
	{
		if (!Candidate.bValid || !Candidate.Actor.IsValid())
		{
			continue;
		}

		if (Candidate.TotalScore > BestScore)
		{
			BestScore = Candidate.TotalScore;
			BestTarget = Candidate.Actor.Get();
		}
	}

	return BestTarget;
}

FCombatTargetInfo UCombatTargetingComponent::GetTargetInfo(AActor* TargetActor) const
{
	FCombatTargetInfo Info;

	const AActor* OwnerActor = GetOwner();
	const ICombatTargetable* Targetable = Cast<ICombatTargetable>(TargetActor);

	if (!IsValid(TargetActor) || !IsValid(OwnerActor) || !Targetable)
	{
		return Info;
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector TargetPoint = Targetable->GetCombatTargetPoint();
	const FVector ToTarget = TargetPoint - OwnerLocation;
	const FVector ToTarget2D(ToTarget.X, ToTarget.Y, 0.f);

	Info.Target = TargetActor;
	Info.TargetPoint = TargetPoint;
	Info.ToTarget = ToTarget;
	Info.ToTarget2D = ToTarget2D.GetSafeNormal();
	Info.Distance = ToTarget2D.Size();
	Info.bIsValid = true;

	return Info;
}
FCombatTargetInfo UCombatTargetingComponent::GetPreferredTargetInfo() const
{
	return GetTargetInfo(PreferredTarget.Get());
}


FCombatTargetInfo UCombatTargetingComponent::GetActiveCombatTargetInfo() const
{
	return GetTargetInfo(ActiveCombatTarget.Get());
}

bool UCombatTargetingComponent::TryBeginAttackCommitWindow()
{
	AActor* CurrentActive = ActiveCombatTarget.Get();

	if (!IsValid(CurrentActive))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("TryBeginAttackCommitWindow failed: No valid ActiveCombatTarget"));
		return false;
	}

	const ECombatTargetValidity Validity = EvaluateActiveTargetValidity(CurrentActive);
	if (Validity == ECombatTargetValidity::ECTV_HardInvalid)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("TryBeginAttackCommitWindow failed: ActiveCombatTarget is hard invalid | Target=%s"),
			*GetNameSafe(CurrentActive));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World) return false;
	
	World->GetTimerManager().ClearTimer(AttackCommitTimerHandle);
	bAttackCommitWindowActive = true;
	World->GetTimerManager().SetTimer(
		AttackCommitTimerHandle,
		this,
		&UCombatTargetingComponent::EndAttackCommitWindow,
		AttackCommitWindowDuration,
		false
	);


	UE_LOG(LogTemp, Warning,
		TEXT("AttackCommitWindow started | ActiveCombatTarget=%s | Duration=%.2f"),
		*GetNameSafe(CurrentActive),
		AttackCommitWindowDuration);

	return true;
}

void UCombatTargetingComponent::EndAttackCommitWindow()
{
	bAttackCommitWindowActive = false;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AttackCommitTimerHandle);
	}

	UE_LOG(LogTemp, Warning,
		TEXT("AttackCommitWindow ended"));
}

void UCombatTargetingComponent::BeginAttackCommitTarget()
{

	if (AActor* CurrentActive = ActiveCombatTarget.Get(); !IsValid(CurrentActive))
	{
		AttackCommittedTarget = nullptr;
		return;
	}
	else
	{
		AttackCommittedTarget = CurrentActive;

		UE_LOG(LogTemp, Warning,
			TEXT("BeginAttackCommitTarget | CommittedTarget=%s"),
			*GetNameSafe(AttackCommittedTarget.Get()));
	}

}

void UCombatTargetingComponent::ClearAttackCommitTarget()
{
	UE_LOG(LogTemp, Warning,
		TEXT("ClearAttackCommitTarget | OldTarget=%s"),
		*GetNameSafe(AttackCommittedTarget.Get()));
	AttackCommittedTarget = nullptr;
}

void UCombatTargetingComponent::ResetActiveTargetGraceState()
{
	bActiveTargetInGracePeriod = false;
	ActiveTargetInvalidElapsed = 0.f;
}

AActor* UCombatTargetingComponent::GetPreferredTarget() const
{
	return PreferredTarget.Get();
}

bool UCombatTargetingComponent::HasPreferredTarget() const
{
	return PreferredTarget.IsValid();
}

AActor* UCombatTargetingComponent::GetActiveCombatTarget() const
{
	return ActiveCombatTarget.Get();
}

bool UCombatTargetingComponent::HasActiveCombatTarget() const
{
	return ActiveCombatTarget.IsValid();
}

AActor* UCombatTargetingComponent::GetAttackCommittedTarget() const
{
	return AttackCommittedTarget.Get();
}

bool UCombatTargetingComponent::HasAttackCommittedTarget() const
{
	return AttackCommittedTarget.IsValid();
}

FCombatTargetInfo UCombatTargetingComponent::GetAttackCommittedTargetInfo() const
{
	return GetTargetInfo(AttackCommittedTarget.Get());
}

void UCombatTargetingComponent::StartScanTimer()
{
	UWorld* World = GetWorld();
	if (!World || World->GetTimerManager().IsTimerActive(ScanTimerHandle)) return;

	World->GetTimerManager().SetTimer(ScanTimerHandle,
		this,
		&UCombatTargetingComponent::HandleScanTimerElapsed,
		ScanInterval,
		true
	);
}

void UCombatTargetingComponent::StopScanTimer()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(ScanTimerHandle);
}

void UCombatTargetingComponent::HandleScanTimerElapsed()
{
	UpdateTargeting();
}


void UCombatTargetingComponent::DebugDraw() const
{

	const AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();

	if (!OwnerActor || !World)
	{
		return;
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();

	DrawDebugSphere(
		World,
		OwnerLocation,
		ScanRadius,
		24,
		FColor::Silver,
		false,
		ScanInterval + 0.02f
	);

	for (const FCombatTargetCandidate& Candidate : CachedCandidates)
	{
		if (!Candidate.Actor.IsValid())
		{
			continue;
		}

		const AActor* CandidateActor = Candidate.Actor.Get();
		const FVector CandidateLocation = CandidateActor->GetActorLocation();

		const bool bIsPreferredTarget = (PreferredTarget.Get() == CandidateActor);
		const bool bIsActiveTarget = (ActiveCombatTarget.Get() == CandidateActor);
		const bool bIsGraceActiveTarget = bIsActiveTarget && bActiveTargetInGracePeriod;

		FColor SphereColor = FColor::Yellow;
		float SphereRadius = 28.f;

		if (bIsPreferredTarget && bIsActiveTarget)
		{
			SphereColor = bIsGraceActiveTarget ? FColor::Blue : FColor::Cyan;
			SphereRadius = 46.f;
		}
		else if (bIsActiveTarget)
		{
			SphereColor = bIsGraceActiveTarget ? FColor::Blue : FColor::Cyan;
			SphereRadius = 42.f;
		}
		else if (bIsPreferredTarget)
		{
			SphereColor = FColor::Red;
			SphereRadius = 38.f;
		}

		DrawDebugSphere(
			World,
			CandidateLocation,
			SphereRadius,
			12,
			SphereColor,
			false,
			ScanInterval + 0.02f
		);

		FString TargetFlags;

		if (bIsPreferredTarget && bIsActiveTarget)
		{
			TargetFlags = bIsGraceActiveTarget ? TEXT("PA GRACE") : TEXT("PA");
		}
		else if (bIsActiveTarget)
		{
			TargetFlags = bIsGraceActiveTarget ? TEXT("A GRACE") : TEXT("A");
		}
		else if (bIsPreferredTarget)
		{
			TargetFlags = TEXT("P");
		}
		else
		{
			TargetFlags = TEXT("-");
		}

		const FString DebugText = FString::Printf(
			TEXT("%s\nScore: %.2f\nDist: %.0f\nD: %.2f F: %.2f C: %.2f SC: %.2f"),
			*TargetFlags,
			Candidate.TotalScore,
			Candidate.Distance,
			Candidate.DistanceScore,
			Candidate.FacingScore,
			Candidate.CameraAlignmentScore,
			Candidate.ScreenCenterScore
		);

		DrawDebugString(
			World,
			CandidateLocation + FVector(0.f, 0.f, 110.f),
			DebugText,
			nullptr,
			SphereColor,
			ScanInterval + 0.02f,
			false
		);
	}

	AActor* CurrentActiveTarget = ActiveCombatTarget.Get();
	const bool bActiveNotInCandidates =
		IsValid(CurrentActiveTarget) && FindCandidateForActor(CurrentActiveTarget) == nullptr;

	if (bActiveNotInCandidates)
	{
		const ICombatTargetable* Targetable = Cast<ICombatTargetable>(CurrentActiveTarget);
		if (Targetable)
		{
			const FVector ActiveTargetPoint = Targetable->GetCombatTargetPoint();
			const FColor ActiveColor = bActiveTargetInGracePeriod ? FColor::Blue : FColor::Cyan;

			DrawDebugSphere(
				World,
				ActiveTargetPoint,
				44.f,
				12,
				ActiveColor,
				false,
				ScanInterval + 0.02f
			);

			const FString ActiveDebugText = FString::Printf(
				TEXT("%s\nNot In Candidates"),
				bActiveTargetInGracePeriod ? TEXT("A GRACE") : TEXT("A")
			);

			DrawDebugString(
				World,
				ActiveTargetPoint + FVector(0.f, 0.f, 130.f),
				ActiveDebugText,
				nullptr,
				ActiveColor,
				ScanInterval + 0.02f,
				false
			);
		}
	}
}

void UCombatTargetingComponent::DebugLogSelection() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== CombatTargeting Scan: %s ==="), *GetNameSafe(OwnerActor));
	UE_LOG(LogTemp, Warning, TEXT("TargetingState: %d"), static_cast<uint8>(TargetingState));
	UE_LOG(LogTemp, Warning, TEXT("Candidates: %d"), CachedCandidates.Num());

	for (const FCombatTargetCandidate& Candidate : CachedCandidates)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("  %s | Score=%.2f | Dist=%.0f | DistScore=%.2f | FacingScore=%.2f | CameraScore=%.2f | ScreenCenterScore=%.2f"),
			*GetNameSafe(Candidate.Actor.Get()),
			Candidate.TotalScore,
			Candidate.Distance,
			Candidate.DistanceScore,
			Candidate.FacingScore,
			Candidate.CameraAlignmentScore,
			Candidate.ScreenCenterScore
		);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("PreferredTarget: %s | ActiveCombatTarget: %s | Grace=%d | InvalidElapsed=%.2f / %.2f"),
		*GetNameSafe(PreferredTarget.Get()),
		*GetNameSafe(ActiveCombatTarget.Get()),
		bActiveTargetInGracePeriod,
		ActiveTargetInvalidElapsed,
		ActiveTargetGracePeriod
	);
}
