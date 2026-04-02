#include "Enemy/EnemyAnimInstance.h"
#include "Enemy/Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Enemy = Cast<AEnemy>(TryGetPawnOwner());
	if (Enemy)
	{
		EnemyMovement = Enemy->GetCharacterMovement();
	}

}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Enemy)
	{
		DeathPose = Enemy->GetDeathPose();
		EnemyState = Enemy->GetEnemyState();
	}

	if (EnemyMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(EnemyMovement->Velocity);
	}
}
