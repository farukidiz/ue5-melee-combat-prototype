#include "Components/CombatAttackComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Characters/BaseCharacter.h"
#include "Data/CombatAttackSetDataAsset.h"
#include "Interfaces/CombatTargetable.h"

UCombatAttackComponent::UCombatAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatAttackComponent] Owner is not ABaseCharacter: %s"), *GetNameSafe(GetOwner()));
		return;
	}

	if (UAnimInstance* AnimInstance = OwnerCharacter->GetCharacterAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &UCombatAttackComponent::HandleMontageEnded);
	}

}

bool UCombatAttackComponent::TryStartAttack()
{
	if (!CanStartAttack())
	{
		return false;
	}

	AActor* CombatTarget = GetCurrentCombatTarget();
	if (!CombatTarget)
	{

		if (const FCombatAttackDefinition* SelectedFreeAttack = SelectFreeAttack())
		{
			return StartAttackFromDefinition(*SelectedFreeAttack);
		}

		return false;
	}

	const float Distance = GetDistanceToTarget2D(CombatTarget);
	if (Distance < 0.f)
	{
		if (bEnableAttackDebugLogs)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[CombatAttackComponent] TryStartAttack failed: invalid target distance"));
		}
		return false;
	}

	const FCombatAttackDefinition* SelectedAttack = SelectInitialAttack(Distance);
	if (!SelectedAttack)
	{
		if (bEnableAttackDebugLogs)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[CombatAttackComponent] No suitable attack found | Target=%s | Distance=%.2f"),
				*GetNameSafe(CombatTarget),
				Distance);
		}
		return false;
	}

	return StartAttackFromDefinition(*SelectedAttack);
}

bool UCombatAttackComponent::CanStartAttack() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	if (!AttackSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatAttackComponent] AttackSet is null on %s"), *GetNameSafe(GetOwner()));
		return false;
	}

	if (bAttackInProgress)
	{
		if (bEnableAttackDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CombatAttackComponent] Attack already in progress"));
		}
		return false;
	}

	return true;
}

AActor* UCombatAttackComponent::GetCurrentCombatTarget() const
{
	return OwnerCharacter ? OwnerCharacter->GetCombatTargetActor() : nullptr;
}

float UCombatAttackComponent::GetDistanceToTarget2D(AActor* Target) const
{
	if (!OwnerCharacter || !IsValid(Target))
	{
		return -1.f;
	}

	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	const FVector Delta = TargetLocation - OwnerLocation;
	const FVector Delta2D(Delta.X, Delta.Y, 0.f);

	return Delta2D.Size();
}

const FCombatAttackDefinition* UCombatAttackComponent::SelectInitialAttack(float Distance) const
{
	if (const FCombatAttackDefinition* CloseCombo = SelectCloseComboAttack(Distance))
	{
		return CloseCombo;
	}

	return SelectNonCloseAttack(Distance);
}

const FCombatAttackDefinition* UCombatAttackComponent::SelectCloseComboAttack(float Distance) const
{
	if (!AttackSet) return nullptr;

	const int32 NextComboIndex = CurrentComboIndex + 1;

	for (const FCombatAttackDefinition& AttackDef : AttackSet->Attacks)
	{
		bool bCloseComboCondition = AttackDef.Category == ECombatAttackCategory::ECAC_CloseCombo &&
									AttackDef.ComboIndex == NextComboIndex &&
									AttackDef.Montage &&
									(Distance == -1.f ? true : IsAttackInDistance(AttackDef, Distance));

		if (bCloseComboCondition)
		{
			return &AttackDef;
		}

	}

	return nullptr;
}

const FCombatAttackDefinition* UCombatAttackComponent::SelectNonCloseAttack(float Distance) const
{
	if (!AttackSet) return nullptr;

	TArray<const FCombatAttackDefinition*> Candidates;

	for (const FCombatAttackDefinition& AttackDef : AttackSet->Attacks)
	{
		bool bAttackCondition = AttackDef.Category != ECombatAttackCategory::ECAC_CloseCombo &&
								AttackDef.Montage &&
								IsAttackInDistance(AttackDef, Distance);

		if (bAttackCondition)
		{
			Candidates.Add(&AttackDef);
		}
	}

	if (Candidates.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, Candidates.Num() - 1);
	return Candidates[RandomIndex];
}

const FCombatAttackDefinition* UCombatAttackComponent::SelectFreeAttack() const
{
	return SelectCloseComboAttack();
}

bool UCombatAttackComponent::IsAttackInDistance(const FCombatAttackDefinition& AttackDef, float Distance) const
{
	return Distance >= AttackDef.MinDistance && Distance <= AttackDef.MaxDistance;
}

bool UCombatAttackComponent::StartAttackFromDefinition(const FCombatAttackDefinition& AttackDef)
{
	if (!OwnerCharacter || !AttackDef.Montage)
	{
		return false;
	}

	const FName StartSection = AttackDef.AttackSection.IsNone()
		? FName(TEXT("Default"))
		: AttackDef.AttackSection;

	const bool bPlayed = OwnerCharacter->PlayMontageSection(AttackDef.Montage, StartSection);
	if (!bPlayed)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CombatAttackComponent] Failed to play montage | AttackID=%s | Montage=%s"),
			*AttackDef.AttackID.ToString(),
			*GetNameSafe(AttackDef.Montage));
		return false;
	}

	ActiveAttackDefinition = &AttackDef;
	ActiveMontage = AttackDef.Montage;
	ActiveTarget = GetCurrentCombatTarget();
	CurrentComboIndex = GetComboStage(AttackDef);
	bAttackInProgress = true;

	if (bEnableAttackDebugLogs)
	{
		const float Distance = GetDistanceToTarget2D(ActiveTarget.Get());

		UE_LOG(LogTemp, Warning,
			TEXT("[CombatAttackComponent] Attack Started | AttackID=%s | Category=%d | Target=%s | Distance=%.2f | Montage=%s"),
			*AttackDef.AttackID.ToString(),
			static_cast<uint8>(AttackDef.Category),
			*GetNameSafe(ActiveTarget.Get()),
			Distance,
			*GetNameSafe(AttackDef.Montage));
	}

	return true;
}

bool UCombatAttackComponent::PlayExitSectionOfCurrentAttack()
{
	if (!OwnerCharacter || !ActiveMontage || !ActiveAttackDefinition)
	{
		return false;
	}

	const FName ExitSection = ActiveAttackDefinition->ExitSection.IsNone()
		? FName(TEXT("Exit"))
		: ActiveAttackDefinition->ExitSection;

	UE_LOG(LogTemp, Warning, TEXT("Exit Section Starting"))
	return OwnerCharacter->JumpToMontageSection(ActiveMontage, ExitSection);
}

void UCombatAttackComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!IsManagedMontage(Montage))
	{
		return;
	}

	if (bEnableAttackDebugLogs)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CombatAttackComponent] Managed montage ended | Montage=%s | Interrupted=%d"),
			*GetNameSafe(Montage),
			bInterrupted);
	}

	ResetAttackState();
	OnManagedAttackFinished.Broadcast(Montage, bInterrupted);
}

bool UCombatAttackComponent::IsManagedMontage(UAnimMontage* Montage) const
{
	return ActiveMontage && Montage == ActiveMontage;
}

void UCombatAttackComponent::ResetAttackState()
{
	bAttackInProgress = false;
	bComboWindowOpen = false;
	bBufferedNextAttack = false;
	CurrentComboIndex = 0;

	ActiveMontage = nullptr;
	ActiveTarget = nullptr;
	ActiveAttackDefinition = nullptr;
}

bool UCombatAttackComponent::ShouldUseFreeAttack()
{
	return GetDistanceToTarget2D(GetCurrentCombatTarget()) > GetMaxAttackDistance();
}

float UCombatAttackComponent::GetMaxAttackDistance() const
{
	if (!AttackSet) return -1.f;

	float MaxHeavyDistance = -1.f;

	for (const FCombatAttackDefinition& AttackDef : AttackSet->Attacks)
	{
		if (AttackDef.Category == ECombatAttackCategory::ECAC_Heavy && AttackDef.Montage)
		{
			MaxHeavyDistance = FMath::Max(MaxHeavyDistance, AttackDef.MaxDistance);
		}
	}

	return MaxHeavyDistance;
}

int32 UCombatAttackComponent::GetComboStage(const FCombatAttackDefinition& Attack) const
{
	if (Attack.Category == ECombatAttackCategory::ECAC_CloseCombo)
	{
		return Attack.ComboIndex;
	}

	return 0;
}

void UCombatAttackComponent::NotifyAttackInput()
{
	if (!bAttackInProgress) return;
	if (!bComboWindowOpen) return;
	if (CurrentComboIndex <= 0) return;

	bBufferedNextAttack = true;

	if (bEnableAttackDebugLogs)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CombatAttackComponent] Combo input buffered | CurrentComboIndex=%d"),
			CurrentComboIndex);
	}
}

void UCombatAttackComponent::HandleComboDecision()
{
	if (CurrentComboIndex <= 0 || !bAttackInProgress)
	{
		return;
	}

	if (bBufferedNextAttack)
	{
		SetCurrentComboIndex();
		const FCombatAttackDefinition* NextAttack = SelectCloseComboAttack();
		bBufferedNextAttack = false;

		if (NextAttack)
		{
			bComboWindowOpen = false;
			StartAttackFromDefinition(*NextAttack);
			return;
		}
	}

	bBufferedNextAttack = false;
	bComboWindowOpen = false;
	PlayExitSectionOfCurrentAttack();
}

void UCombatAttackComponent::OpenComboWindow()
{
	if (CurrentComboIndex <= 0) return;

	bComboWindowOpen = true;

	if (bEnableAttackDebugLogs)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CombatAttackComponent] Combo window opened | ComboIndex=%d"),
			CurrentComboIndex);
	}
}

void UCombatAttackComponent::CloseComboWindow()
{
	bComboWindowOpen = false;

	if (bEnableAttackDebugLogs)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CombatAttackComponent] Combo window closed | ComboIndex=%d"),
			CurrentComboIndex);
	}
}

void UCombatAttackComponent::SetCurrentComboIndex()
{
	if (CurrentComboIndex == 3)
	{
		CurrentComboIndex = 0;
	}
}