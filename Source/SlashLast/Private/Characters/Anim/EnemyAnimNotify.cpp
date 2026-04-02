#include "Characters/Anim/EnemyAnimNotify.h"
#include "Enemy/Enemy.h"

void UEnemyAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp)
        return;

    AEnemy* Enemy = Cast<AEnemy>(MeshComp->GetOwner());
    if (!Enemy)
        return;


    switch (CollisionNotify)
    {
    case EEnemyCollisionNotify::ECN_DisableBoxCollision:
        Enemy->SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
        break;

    case EEnemyCollisionNotify::ECN_EnableBoxCollision:
        Enemy->SetWeaponCollisionEnabled(ECollisionEnabled::QueryOnly);

    }

}
