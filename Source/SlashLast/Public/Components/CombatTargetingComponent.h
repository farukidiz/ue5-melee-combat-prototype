#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTargetingComponent.generated.h"

struct FCombatTargetInfo;
struct FCombatTargetCandidate;
enum class ECombatTargetingState : uint8;
enum class ECombatTargetValidity : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreferredTargetChanged, AActor*, OldTarget, AActor*, NewTarget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActiveCombatTargetChanged, AActor*, OldTarget, AActor*, NewTarget);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASHLAST_API UCombatTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatTargetingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	AActor* GetPreferredTarget() const;
	bool HasPreferredTarget() const;
	FCombatTargetInfo GetPreferredTargetInfo() const;

	AActor* GetActiveCombatTarget() const;
	bool HasActiveCombatTarget() const;
	FCombatTargetInfo GetActiveCombatTargetInfo() const;

	AActor* GetAttackCommittedTarget() const;
	bool HasAttackCommittedTarget() const;
	FCombatTargetInfo GetAttackCommittedTargetInfo() const;

	void BeginAttackCommitTarget();
	void ClearAttackCommitTarget();


	bool TryBeginAttackCommitWindow();

	UPROPERTY(BlueprintAssignable, Category = "Targeting")
	FOnPreferredTargetChanged OnPreferredTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Targeting")
	FOnActiveCombatTargetChanged OnActiveCombatTargetChanged;

protected:

	void UpdateTargeting();
	void ScanForCandidates();
	void EvaluatePreferredTarget();
	void SetPreferredTarget(AActor* NewTarget);


	void UpdateActiveCombatTarget();
	void SetActiveCombatTarget(AActor* NewTarget);
	const FCombatTargetCandidate* FindCandidateForActor(AActor* Actor) const;
	float GetCandidateScore(AActor* Actor) const;

	bool IsActorTargetable(AActor* Actor) const;
	bool TryBuildCandidate(AActor* Actor, FCombatTargetCandidate& OutCandidate) const;

	float CalculateFacingDot(const FVector& OwnerForward, const FVector& ToTargetDir) const;
	float CalculateCameraDot(const FVector& ToTargetDir) const;
	float CalculateScreenCenterScore(const FVector& WorldLocation) const;
	float CalculateDistanceScore(float Distance) const;
	float CalculateFacingScore(float FacingDot) const;
	float CalculateCameraAlignmentScore(float CameraDot) const;

	FVector GetCameraForwardVector() const;
	FVector GetOwnerLocation() const;
	FVector GetOwnerForwardVector() const;
	void DebugDraw() const;
	void DebugLogSelection() const;

	UPROPERTY(EditAnywhere, Category = "Targeting|Scan")
	float ScanRadius = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Scan")
	float ScanInterval = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Score")
	float DistanceWeight = 0.55f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Score")
	float FacingWeight = 0.45f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Score")
	float ScreenCenterWeight = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Score")
	float CameraAlignmentWeight = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Score")
	float MinFacingDot = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Score")
	float MinCameraDot = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Active")
	float ActiveSoftSwitchScoreThreshold = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Active")
	float ActiveValidSwitchScoreThreshold = 0.10f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Screen")
	bool bUseScreenCenterScore = true;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug")
	bool bEnableDebugLogs = true;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug")
	bool bEnableDebugDraw = true;

	UPROPERTY(VisibleAnywhere, Category = "Targeting")
	ECombatTargetingState TargetingState;

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	TWeakObjectPtr<AActor> PreferredTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	TWeakObjectPtr<AActor> ActiveCombatTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Targeting|Commit")
	TWeakObjectPtr<AActor> AttackCommittedTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	TArray<FCombatTargetCandidate> CachedCandidates;

	UPROPERTY(EditAnywhere, Category = "Targeting|Active")
	float ActiveTargetGracePeriod = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Targeting|Active")
	bool bActiveTargetInGracePeriod = false;

	UPROPERTY(VisibleAnywhere, Category = "Targeting|Active")
	float ActiveTargetInvalidElapsed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Targeting|Active")
	float ActiveCombatTargetLastKnownScore = -1.f;


private:

	AActor* SelectPreferredTarget() const;

	void StartScanTimer();
	void StopScanTimer();
	void HandleScanTimerElapsed();
	void EndAttackCommitWindow();

	bool SearchActors(TArray<AActor*>& OutActors) const;

	ECombatTargetValidity EvaluateActiveTargetValidity(AActor* Actor) const;
	void HandleValidActiveTarget(AActor* CurrentActive, AActor* CurrentPreferred);
	void HandleHardInvalidActiveTarget(AActor* CurrentPreferred);
	void HandleSoftInvalidActiveTarget(AActor* CurrentActive, AActor*);
	void HandleCommitWindowActiveTarget(ECombatTargetValidity Validity);
	void SwitchActiveCombatTarget(AActor* NewTarget, const TCHAR* Reason);
	void ResetActiveTargetGraceState();

	FCombatTargetInfo GetTargetInfo(AActor* TargetActor) const;

	FTimerHandle ScanTimerHandle;
	FTimerHandle AttackCommitTimerHandle;

	bool bAttackCommitWindowActive = false;

	UPROPERTY(EditAnywhere, Category = "Targeting|Commit")
	float AttackCommitWindowDuration = 0.25f;

	bool IsAttackCommitWindowActive() const { return bAttackCommitWindowActive; }

};


