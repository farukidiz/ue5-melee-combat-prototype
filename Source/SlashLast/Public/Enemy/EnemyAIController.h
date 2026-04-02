#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"


enum class EEnemyState : uint8;
UCLASS()
class SLASHLAST_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:

	void MoveToTarget(AActor* Target);
	void ClearPatrolTimer();
	void ClearAttackTimer();
	void EvaluateStateFromContext();

	void HandleTargetSeen(APawn* SeenPawn);
	void HandleTargetLost(APawn* SeenPawn);

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:

	UFUNCTION()

	void EnterPatrolState();
	void EnterChaseState(AActor* Target);
	void EnterAttackState();

	void MoveToCurrentPatrolTarget();
	void StartInitialPatrol();
	void LoseInterest();
	void ExecuteAttack();

	void ScheduleAttack();
	void ScheduleNextPatrolMove();

	void StartStateEvaluationTimer();
	void ClearStateEvaluationTimer();

	bool IsEnemyValid() const;
	bool IsCombatTargetDead() const;
	bool IsEnemyOnBusyState() const;

	void HandleStateEvaluationTimerElapsed();

	FTimerHandle PatrolTimer;
	FTimerHandle AttackTimer;
	FTimerHandle StateEvaluationTimer;

	class AEnemy* ControlledEnemy;

};
