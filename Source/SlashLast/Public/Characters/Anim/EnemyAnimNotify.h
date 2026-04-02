#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EnemyAnimNotify.generated.h"


UENUM(BlueprintType)
enum class EEnemyCollisionNotify : uint8 {

    ECN_EnableBoxCollision UMETA(DisplayName = "EnableBoxCollision"),
    ECN_DisableBoxCollision UMETA(DisplayName = "DisableBoxCollision")
};
UCLASS()
class SLASHLAST_API UEnemyAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
protected:

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Collision")
    EEnemyCollisionNotify CollisionNotify;
};
