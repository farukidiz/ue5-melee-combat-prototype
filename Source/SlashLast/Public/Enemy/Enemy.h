#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Interfaces/CombatTargetable.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class AEnemyAIController;
class UAISenseConfig_Sight;
class UAIPerceptionComponent;
struct FAIStimulus;

enum class EEnemyState : uint8;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEnemySight, APawn*, bool);

UCLASS()
class SLASHLAST_API AEnemy : public ABaseCharacter, public ICombatTargetable
{
	GENERATED_BODY()


public:

	AEnemy();

	AActor* ChooseNewPatrolTarget();

		/* <AActor*> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
		/* </AActor*>*/

	/* <IHitInterface> */
	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */


	/* <ICombatTargetable> */
	virtual bool CanBeTargeted() const override;
	virtual bool IsDeadForTargeting() const override;
	virtual FVector GetCombatTargetPoint() const override;
	/* <ICombatTargetable> */

	/*  AI Behaviour */
	void HideHealthBar();
	void ShowHealthBar();
	void WalkPatrollingSpeed();
	void WalkChasingSpeed();
	bool IsChasing() const;
	bool IsAttacking() const;
	bool IsPatrolling() const;
	bool IsEngaged() const;
	bool IsHitReact() const;
	bool IsDead() const;

	bool IsOutsideLoseInterestRadius() const;
	bool IsOutsideAttackRadius() const;
	bool IsOutsideLoseSightRadius() const;
	bool IsInsideAttackRadius() const;
	bool CanAttack() const override;

	FORCEINLINE const EDeathPose GetDeathPose() const { return DeathPose; }
	FORCEINLINE const EEnemyState GetEnemyState() const { return EnemyState; }
	FORCEINLINE float GetPatrolWaitMin() const { return PatrolWaitMin; }
	FORCEINLINE float GetPatrolWaitMax() const { return PatrolWaitMax; }
	FORCEINLINE float GetAttackMin() const { return AttackMin; }
	FORCEINLINE float GetAttackMax() const { return AttackMax; }
	FORCEINLINE double GetAttackRadius() const { return AttackRadius; }
	FORCEINLINE double GetPatrolRadius() const { return PatrolRadius; }
	FORCEINLINE double GetAcceptableRadius() const { return AcceptableRadius; }
	FORCEINLINE AActor* GetCombatTarget() const { return CombatTarget; }
	FORCEINLINE void SetCombatTarget(AActor* Target) { CombatTarget = Target; }
	FORCEINLINE void SetEnemyState(EEnemyState State) { EnemyState = State; }
	

	virtual void Attack() override;

	FOnEnemySight OnEnemySight;

protected:
	/* <AActor*> */
	virtual void BeginPlay() override;
	/* </AActor*> */

	/* <ABaseCharacter*> */
	void HandleDamage(float DamageAmount) override;
	void MontageEnd(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void Die() override;
	void SpawnPickupItem();
	void SpawnPotion();
	void SpawnSoul();
	/* </ABaseCharacter*> */

private:

	/*  AI Behaviour */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	bool IsTargetInRange(AActor* Target, double Radius) const;

	void DisableAIMovement();

	void InitializeEnemy();
	void SpawnDefaultWeapon();
	void SetSightConfig();
	void SetEnemyHitReactState();

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* PerceptionComp;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	AEnemyAIController* EnemyController;


	UPROPERTY(EditAnywhere, Category = Combat)
	float SightRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float LoseSightRadius = 1400.f;

	float LoseInterestRadius;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AttackRadius = 200.0;

	UPROPERTY(EditAnywhere, Category = Combat)
	double PatrolRadius = 60.0;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AcceptableRadius = 100.0;

	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditInstanceOnly, Category = Combat)
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = Combat)
	TArray<AActor*> PatrolTargets;


	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 1.0f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 7.f;

	UPROPERTY(EditInstanceOnly, Category = Combat)
	float PatrolWaitMin = 5.f;

	UPROPERTY(EditInstanceOnly, Category = Combat)
	float PatrolWaitMax = 10.f;

	UPROPERTY(EditAnywhere, Category = Pickups)
	float PotionWeight = 20.f;

	UPROPERTY(EditAnywhere, Category = Pickups)
	float SoulWeight = 80.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class ASoul> SoulClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class APotion> PotionClass;

	EEnemyState EnemyState;

};
