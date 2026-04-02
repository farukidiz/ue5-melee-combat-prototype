#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_MotionWarping.h"
#include "EnemyAnimNotifyState_MW.generated.h"


UENUM(BlueprintType)
enum class EMotionWarpNotifyMode : uint8
{
	EMN_TranslationOnly UMETA(DisplayName = "Translation Only"),
	EMN_RotationOnly UMETA(DisplayName = "Rotation Only"),
	EMN_None UMETA(DisplayName = "None"),

};

UCLASS(meta = (DisplayName = "Enemy Motion Warp State"))
class SLASHLAST_API UEnemyAnimNotifyState_MW : public UAnimNotifyState_MotionWarping
{
	GENERATED_BODY()

public:

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom")
	EMotionWarpNotifyMode WarpMode = EMotionWarpNotifyMode::EMN_None;

};
