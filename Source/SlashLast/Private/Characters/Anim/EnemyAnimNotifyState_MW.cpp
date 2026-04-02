#include "Characters/Anim/EnemyAnimNotifyState_MW.h"
#include "Enemy/Enemy.h"
#include "Characters/SlashCharacter.h"


void UEnemyAnimNotifyState_MW::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp) return;

	ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());
	if (!Character) return;

	switch (WarpMode)
	{
	case EMotionWarpNotifyMode::EMN_TranslationOnly:
		Character->SetTranslationWarpTarget();
		break;

	case EMotionWarpNotifyMode::EMN_RotationOnly:
		Character->SetRotationWarpTarget();
		break;

	}
}