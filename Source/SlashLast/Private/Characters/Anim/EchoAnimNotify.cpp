#include "Characters/Anim/EchoAnimNotify.h"
#include "Characters/SlashCharacter.h"
#include "Items/Weapons/Weapon.h"

void UEchoAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) 
        return;

    ASlashCharacter* Character = Cast<ASlashCharacter>(MeshComp->GetOwner());
    if (!Character) 
        return;

    if (NotifyId == FName(TEXT("None")))
    {
        switch (EquipNotify)
        {
        case EEquipNotify::EEN_Disarm:
            Character->AttachWeaponToBack();
            break;

        case EEquipNotify::EEN_Arm:
            Character->AttachWeaponToHand();
            break;
        }

        switch (CollisionNotify)
        {
        case ECollisionNotify::ECN_DisableBoxCollision:
            Character->SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
            break;

        case ECollisionNotify::ECN_EnableBoxCollision:
            Character->SetWeaponCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
        }

    }
    else if (NotifyId == FName(TEXT("OpenDecision")))
    {
        Character->ComboWindowStart();
    }
    else if (NotifyId == FName(TEXT("CloseDecision")))
    {
        Character->ComboWindowStop();
    }
    else if (NotifyId == FName(TEXT("Decision")))
    {
        Character->ComboDecision();
    }

}