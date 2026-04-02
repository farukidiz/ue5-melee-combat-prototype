#include "Enemy/EnemyAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Enemy/Enemy.h"
#include "Characters/CharacterTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DebugDrawUtils.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ControlledEnemy = Cast<AEnemy>(InPawn);
    if (ControlledEnemy)
    {

        FTimerHandle InitHandle;
        GetWorldTimerManager().SetTimer(InitHandle,this,&AEnemyAIController::StartInitialPatrol,0.1f,false);
    }
}

void AEnemyAIController::StartInitialPatrol()
{
    EnterPatrolState();
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{

    Super::OnMoveCompleted(RequestID, Result);
    
    if (!IsEnemyValid()) return;
    if (Result.Code != EPathFollowingResult::Success) return;

    if (ControlledEnemy->IsPatrolling())
    {
        ScheduleNextPatrolMove();
    }
    else
    {
        EvaluateStateFromContext();
    }
}

void AEnemyAIController::HandleTargetSeen(APawn* SeenPawn)
{
    if (!IsEnemyValid() || !SeenPawn || SeenPawn->ActorHasTag(FName("Death"))) return;

    ControlledEnemy->SetCombatTarget(SeenPawn);
    EvaluateStateFromContext();

}

void AEnemyAIController::HandleTargetLost(APawn* SeenPawn)
{
    if (!IsEnemyValid()) return;

    EvaluateStateFromContext();

}

void AEnemyAIController::EvaluateStateFromContext()
{
    if (!IsEnemyValid() || IsEnemyOnBusyState()) return;

    if (IsCombatTargetDead())
    {
        LoseInterest();
        EnterPatrolState();
        return;
    }

    if (ControlledEnemy->IsOutsideLoseInterestRadius())
    {

        LoseInterest();
        EnterPatrolState();
        return;
    }

    if (ControlledEnemy->IsInsideAttackRadius())
    {
            EnterAttackState();
        return;
    }

    if (AActor* Target = ControlledEnemy->GetCombatTarget())
    {

            EnterChaseState(Target);
        return;
    }

        EnterPatrolState();
}

void AEnemyAIController::EnterPatrolState()
{
    if (!IsEnemyValid()) return;
    ClearAttackTimer();
    ClearPatrolTimer();
    ClearStateEvaluationTimer();
    StopMovement();

    ControlledEnemy->SetEnemyState(EEnemyState::EES_Patrolling);
    ControlledEnemy->WalkPatrollingSpeed();

    MoveToCurrentPatrolTarget();
}

void AEnemyAIController::EnterChaseState(AActor* Target)
{

    if (!IsEnemyValid() || !Target) return;

    if (ControlledEnemy->GetEnemyState() == EEnemyState::EES_Chasing &&
        ControlledEnemy->GetCombatTarget() == Target)
    {
        return;
    }

    ClearPatrolTimer();
    ClearAttackTimer();
    StartStateEvaluationTimer();

    ControlledEnemy->SetCombatTarget(Target);
    ControlledEnemy->SetEnemyState(EEnemyState::EES_Chasing);
    ControlledEnemy->WalkChasingSpeed();

    MoveToTarget(Target);
}

void AEnemyAIController::EnterAttackState()
{
    if (!IsEnemyValid()) return;

    if (ControlledEnemy->GetEnemyState() == EEnemyState::EES_Attacking) return;

    ClearAttackTimer();
    ClearPatrolTimer();
    ClearStateEvaluationTimer();
    StopMovement();

    ControlledEnemy->SetEnemyState(EEnemyState::EES_Attacking);
    ControlledEnemy->WalkChasingSpeed();

    ScheduleAttack();

}

void AEnemyAIController::LoseInterest()
{
    ControlledEnemy->SetCombatTarget(nullptr);
    ControlledEnemy->HideHealthBar();
}

void AEnemyAIController::MoveToCurrentPatrolTarget()
{

    if (!IsEnemyValid()) return;

    MoveToTarget(ControlledEnemy->ChooseNewPatrolTarget());

}

void AEnemyAIController::ExecuteAttack()
{

    if (!IsEnemyValid()) return;

    if (ControlledEnemy->CanAttack())
    {
        ControlledEnemy->Attack();
    }   
    else
    {
        EvaluateStateFromContext();
    }
}

void AEnemyAIController::MoveToTarget(AActor* Target)
{

    if (!IsEnemyValid() || !Target) return;

    EPathFollowingRequestResult::Type Result = MoveToActor(Target, ControlledEnemy->GetAcceptableRadius());

}

void AEnemyAIController::ScheduleNextPatrolMove()
{
    ClearPatrolTimer();
    float RandomDelayTime = FMath::FRandRange(ControlledEnemy->GetPatrolWaitMin(), ControlledEnemy->GetPatrolWaitMax());

    GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemyAIController::MoveToCurrentPatrolTarget, RandomDelayTime);
}

void AEnemyAIController::ScheduleAttack()
{
    ClearAttackTimer();
    const float AttackDelay = FMath::RandRange(ControlledEnemy->GetAttackMin(), ControlledEnemy->GetAttackMax());

    GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyAIController::ExecuteAttack, AttackDelay);
}

void AEnemyAIController::StartStateEvaluationTimer()
{
    if (!IsEnemyValid() || GetWorldTimerManager().IsTimerActive(StateEvaluationTimer)) return;

    GetWorldTimerManager().SetTimer(StateEvaluationTimer, this, &AEnemyAIController::HandleStateEvaluationTimerElapsed, 0.2f, true);
}

void AEnemyAIController::ClearPatrolTimer()
{
    GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemyAIController::ClearAttackTimer()
{
    GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemyAIController::ClearStateEvaluationTimer()
{
    GetWorldTimerManager().ClearTimer(StateEvaluationTimer);
}
bool AEnemyAIController::IsEnemyValid() const
{
    return ControlledEnemy != nullptr;
}

bool AEnemyAIController::IsCombatTargetDead() const
{
    if (!IsEnemyValid()) return true;

    AActor* Target = ControlledEnemy->GetCombatTarget();
    if (!IsValid(Target)) return true;

    return Target->ActorHasTag(FName("Death"));
}

bool AEnemyAIController::IsEnemyOnBusyState() const
{
    if (!IsEnemyValid()) return true;

    EEnemyState CurrentState = ControlledEnemy->GetEnemyState();
    return  CurrentState == EEnemyState::EES_HitReact || CurrentState == EEnemyState::EES_Dead || CurrentState == EEnemyState::EES_Engaged;
}

void AEnemyAIController::HandleStateEvaluationTimerElapsed()
{
    EvaluateStateFromContext();
}
