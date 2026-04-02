#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimInstance.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/CharacterTypes.h"
#include "MotionWarpingComponent.h"

ABaseCharacter::ABaseCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ABaseCharacter::MontageEnd);
	}
	
}

void ABaseCharacter::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{

	double Theta = CalculateHitAngle(Hitter->GetActorLocation());
	FName SectionName;

	StopAttackMontage();

	if (IsAlive())
	{
		AngleToDirectionName(Theta, SectionName, true);
		PlayMontageSection(HitReactMontage, SectionName);

	}
	else
	{
		AngleToDirectionName(Theta, SectionName, false);
		PlayMontageSection(DeathReactMontage, SectionName);
		DeathSoundEffect(ImpactPoint);

	}
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayEffects(ImpactPoint);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}

}

bool ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName) const
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
		return true;
	}

	return false;
}

bool ABaseCharacter::JumpToMontageSection(UAnimMontage* Montage, const FName& SectionName) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !Montage)
	{
		return false;
	}

	AnimInstance->Montage_JumpToSection(SectionName, Montage);
	return true;
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

void ABaseCharacter::PlayAttackMontage()
{

	if (AttackMontageSections.Num() <= 0) return;

	const int32 MaxSectionIdx = AttackMontageSections.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIdx);
	PlayMontageSection(AttackMontage, AttackMontageSections[Selection]);

}

void ABaseCharacter::PlayEffects(const FVector& ImpactPoint) const
{

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
	
	if (HitParticle && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, ImpactPoint);
	}

}

void ABaseCharacter::DeathSoundEffect(const FVector& ImpactPoint) const
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, ImpactPoint);
	}
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (!EquippedWeapon) return;

	if (CollisionEnabled == ECollisionEnabled::QueryOnly)
	{
		EquippedWeapon->BeginAttackWindow();
	}
	else
	{
		EquippedWeapon->EndAttackWindow();
	}

}

void ABaseCharacter::SetTranslationWarpTarget()
{
	if (!CombatTarget) return;

	const FVector Location = GetActorLocation();
	const FVector TargetLocation = CombatTarget->GetActorLocation();

	FVector TargetToMe = (Location - TargetLocation).GetSafeNormal() * WarpTargetDistance;
	FVector WarpLocation = TargetLocation + TargetToMe;

	SetWarpTarget(FName{ "TranslationTarget" }, WarpLocation);

}

void ABaseCharacter::SetRotationWarpTarget()
{
	if (!CombatTarget) return;

	SetWarpTarget(FName{ "RotationTarget" }, CombatTarget->GetActorLocation());
}

void ABaseCharacter::SetWarpTarget(FName WarpTargetName, FVector TargetLocation)
{
	if (MotionWarping && CombatTarget)
	{
		MotionWarping->AddOrUpdateWarpTargetFromLocation(WarpTargetName, TargetLocation);
	}
}

void ABaseCharacter::ClearMotionWarpTargets()
{
	if (!MotionWarping) return;

	MotionWarping->RemoveWarpTarget(FName("TranslationTarget"));
	MotionWarping->RemoveWarpTarget(FName("RotationTarget"));
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DisableMesh()
{
	GetMesh()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void ABaseCharacter::DisableCharacterMovement()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();               
	GetCharacterMovement()->SetComponentTickEnabled(false);  
}

void ABaseCharacter::EnablePhysicsForBody()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

void ABaseCharacter::Die()
{
	Tags.Add(FName("Death"));
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	StopAnimMontage(AttackMontage);
	DisableCharacterMovement();
	DisableCapsule();
	if (bPhysicsAfterDie)
	{
		EnablePhysicsForBody();
	}
	else
	{
		DisableMesh();
	}

}

void ABaseCharacter::Attack() {}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

double ABaseCharacter::CalculateHitAngle(const FVector& ImpactPoint) const
{
	FVector Forward = GetActorForwardVector();
	FVector ImpactLowered{ ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z };
	FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	double Theta = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, ToHit)));

	if (FVector::CrossProduct(Forward, ToHit).Z < 0)
	{
		Theta *= -1;
	}

	return Theta;

}

const FName& ABaseCharacter::HitDirectionName(double Angle, FName& SectionName) const
{
	SectionName = "FromBack";
	if (Angle >= -45.f && Angle < 45.f)
	{
		SectionName = "FromFront";
	}
	else if (Angle >= 45.f && Angle < 135.f)
	{
		SectionName = "FromRight";
	}
	else if (Angle < -45.f && Angle >= -135.f)
	{
		SectionName = "FromLeft";
	}
	return SectionName;
}
const FName& ABaseCharacter::AngleToDirectionName(double Angle, FName& SectionName, bool IsAlive)
{
	if (IsAlive)
	{
		return HitDirectionName(Angle, SectionName);
	}
	else
	{
		return DeathDirectionName(Angle, SectionName);
	}
}
const FName& ABaseCharacter::DeathDirectionName(double Angle, FName& SectionName)
{
	const int32 PoseNum = FMath::RandRange(1, 3);
	if (Angle >= -90.f && Angle < 90.f)
	{
		SectionName = (*FString::Printf(TEXT("Death_Front%d"), PoseNum));
		switch (PoseNum)
		{
		case 1:
			DeathPose = EDeathPose::EDP_DeathPoseFront1; break;
		case 2:
			DeathPose = EDeathPose::EDP_DeathPoseFront2; break;
		case 3:
			DeathPose = EDeathPose::EDP_DeathPoseFront3; break;
		default:
			break;
		}
	}
	else
	{
		SectionName = (*FString::Printf(TEXT("Death_Back%d"), PoseNum));
		switch (PoseNum)
		{
		case 1:
			DeathPose = EDeathPose::EDP_DeathPoseBack1; break;
		case 2:
			DeathPose = EDeathPose::EDP_DeathPoseBack2; break;
		case 3:
			DeathPose = EDeathPose::EDP_DeathPoseBack3; break;
		default:
			break;
		}
	}

	return SectionName;
}
void ABaseCharacter::MontageEnd(UAnimMontage* Montage, bool bInterrupted)
{

}

UAnimInstance* ABaseCharacter::GetCharacterAnimInstance() const
{
	return GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
}