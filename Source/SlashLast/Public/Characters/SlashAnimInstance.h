#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SlashAnimInstance.generated.h"


enum class ECharacterState : uint8;
enum class EActionState : uint8;
enum class EDeathPose : uint8;

UCLASS()
class SLASHLAST_API USlashAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadOnly)
	class ASlashCharacter* SlashCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	class UCharacterMovementComponent* SlashCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	double GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = States)
	ECharacterState CharacterState;

	UPROPERTY(BlueprintReadOnly, Category = States)
	EActionState ActionState;

	UPROPERTY(BlueprintReadOnly, Category = States)
	EDeathPose DeathPose;

};
