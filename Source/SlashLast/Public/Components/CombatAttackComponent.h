#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatAttackComponent.generated.h"


class ABaseCharacter;
class UAnimMontage;
class UCombatAttackSetDataAsset;
struct FCombatAttackDefinition;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnManagedAttackFinished, UAnimMontage*, bool);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SLASHLAST_API UCombatAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatAttackComponent();

	virtual void BeginPlay() override;

	bool TryStartAttack();

	bool IsAttackInProgress() const { return bAttackInProgress; }

	bool ShouldUseFreeAttack();

	FOnManagedAttackFinished OnManagedAttackFinished;


	void NotifyAttackInput();
	void OpenComboWindow();
	void CloseComboWindow();
	void HandleComboDecision();

protected:

	UPROPERTY(EditAnywhere, Category = "Combat|Attack Data")
	UCombatAttackSetDataAsset* AttackSet = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat|Debug")
	bool bEnableAttackDebugLogs = true;

private:
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void SetCurrentComboIndex();

	bool CanStartAttack() const;
	AActor* GetCurrentCombatTarget() const;
	float GetDistanceToTarget2D(AActor* Target) const;
	float GetMaxAttackDistance() const;
	int32 GetComboStage(const FCombatAttackDefinition& Attack) const;

	const FCombatAttackDefinition* SelectInitialAttack(float Distance) const;
	const FCombatAttackDefinition* SelectCloseComboAttack(float Distance = -1.f) const;
	const FCombatAttackDefinition* SelectNonCloseAttack(float Distance) const;
	const FCombatAttackDefinition* SelectFreeAttack() const;


	bool IsAttackInDistance(const FCombatAttackDefinition& AttackDef, float Distance) const;
	bool StartAttackFromDefinition(const FCombatAttackDefinition& AttackDef);
	bool IsManagedMontage(UAnimMontage* Montage) const;
	void ResetAttackState();

	bool PlayExitSectionOfCurrentAttack();

private:

	UPROPERTY(Transient)
	ABaseCharacter* OwnerCharacter = nullptr;

	UPROPERTY(Transient)
	UAnimMontage* ActiveMontage = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> ActiveTarget = nullptr;

	UPROPERTY(Transient)
	bool bAttackInProgress = false;

	UPROPERTY(Transient)
	bool bComboWindowOpen = false;

	UPROPERTY(Transient)
	bool bBufferedNextAttack = false;

	UPROPERTY(Transient)
	int32 CurrentComboIndex = 0;

	const FCombatAttackDefinition* ActiveAttackDefinition;

};