#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

enum class EDeathPose : uint8;
enum class EEnemyState : uint8;
UCLASS()
class SLASHLAST_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;


	UPROPERTY(BlueprintReadOnly)
	class AEnemy* Enemy;

	UPROPERTY(BlueprintReadOnly, Category = "Death Pose")
	EDeathPose DeathPose;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy State")
	EEnemyState EnemyState;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	class UCharacterMovementComponent* EnemyMovement;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	double GroundSpeed;
};
