#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"


class AWeapon;
class UAnimMontage;
class UAnimInstance;
class UAttributeComponent;
class UMotionWarpingComponent;
enum class EDeathPose : uint8;

UCLASS()
class SLASHLAST_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:

	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	void SetTranslationWarpTarget();
	void SetRotationWarpTarget();
	void ClearMotionWarpTargets();

	UAnimInstance* GetCharacterAnimInstance() const;

	void StopAttackMontage();
	void PlayAttackMontage();
	virtual bool PlayMontageSection(UAnimMontage* Montage, const FName& SectionName) const;
	bool JumpToMontageSection(UAnimMontage* Montage, const FName& SectionName) const;

	FORCEINLINE double GetWarpTargetDistance() { return WarpTargetDistance; }
	FORCEINLINE EDeathPose GetDeathPose() const { return DeathPose; }
	FORCEINLINE AActor* GetCombatTargetActor() const { return CombatTarget; }

protected:

	virtual void BeginPlay() override;

	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;

	UFUNCTION()
	virtual void MontageEnd(UAnimMontage* Montage, bool bInterrupted);
	virtual void PlayEffects(const FVector& ImpactPoint) const;
	virtual void DeathSoundEffect(const FVector& ImpactPoint) const;
	virtual bool IsAlive();

	virtual void Attack();
	virtual bool CanAttack() const { return true; }
	virtual void HandleDamage(float DamageAmount);

	void DisableCapsule();
	void DisableMesh();
	void DisableCharacterMovement();
	void EnablePhysicsForBody();
	virtual void Die();

	virtual double CalculateHitAngle(const FVector& ImpactPoint) const;
	virtual const FName& HitDirectionName(double Angle, FName& SectionName) const;
	virtual const FName& AngleToDirectionName(double Angle, FName& SectionName, bool IsAlive);
	virtual const FName& DeathDirectionName(double Angle, FName& SectionName);

	virtual void SetCombatTargetActor(AActor* NewTarget) { CombatTarget = NewTarget; }

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathReactMontage;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere)
	UMotionWarpingComponent* MotionWarping;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(VisibleInstanceOnly)
	AWeapon* EquippedWeapon = nullptr;

	EDeathPose DeathPose;

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 110.0;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	bool bPhysicsAfterDie = false;

private:

	void SetWarpTarget(FName WarpTargetName, FVector TargetLocation);

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> AttackMontageSections;

};
