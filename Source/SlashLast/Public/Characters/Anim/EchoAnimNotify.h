#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EchoAnimNotify.generated.h"


UENUM(BlueprintType)
enum class EEquipNotify : uint8
{
    EEN_Disarm UMETA(DisplayName = "AttachWeaponToBack"),
    EEN_Arm UMETA(DisplayName = "AttachWeaponToArm"),
};

UENUM(BlueprintType)
enum class ECollisionNotify : uint8 {

    ECN_EnableBoxCollision UMETA(DisplayName = "EnableBoxCollision"),
    ECN_DisableBoxCollision UMETA(DisplayName = "DisableBoxCollision")
};

UCLASS()
class SLASHLAST_API UEchoAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

protected:

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Equip")
    EEquipNotify EquipNotify;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Collision")
    ECollisionNotify CollisionNotify;

    UPROPERTY(EditAnywhere, Category = "NotifyId")
    FName NotifyId = TEXT("None");

};
