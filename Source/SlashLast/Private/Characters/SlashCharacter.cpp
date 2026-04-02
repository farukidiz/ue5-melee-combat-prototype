#include "Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Components/CombatTargetingComponent.h"
#include "Components/CombatAttackComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Items.h"
#include "Items/Weapons/Weapon.h"
#include "Characters/CharacterTypes.h"
#include "Animation/AnimInstance.h"
#include "Data/CombatTargetingTypes.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Items/Soul.h"
#include "Items/Treasures/Treasure.h"
#include "Items/Potion.h"

ASlashCharacter::ASlashCharacter() : ActionState(EActionState::EAS_Unoccupied), CharacterState(ECharacterState::ECS_Unequipped)
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* CharacterMovementControl = GetCharacterMovement();
	CharacterMovementControl->bOrientRotationToMovement = true;
	CharacterMovementControl->RotationRate = FRotator{ 0.f, 450.f, 0.f };


	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	ViewCamera->SetupAttachment(SpringArm);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh(), FName("head"));

	EyeBrows = CreateDefaultSubobject<UGroomComponent>(TEXT("EyeBrows"));
	EyeBrows->SetupAttachment(GetMesh(), FName("head"));

	CombatTargetingComponent = CreateDefaultSubobject<UCombatTargetingComponent>(TEXT("CombatTargetingComponent"));

	CombatAttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("CombatAttackComponent"));

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ASlashCharacter::Dodge);
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);

	return DamageAmount;
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (Attributes && Attributes->IsStaminaNotFull() && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName{ "EngageableTarget" });
	

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		Subsystem->AddMappingContext(SlashContext, 0);
	}


	if (CombatAttackComponent)
	{
		CombatAttackComponent->OnManagedAttackFinished.AddUObject(
			this,
			&ASlashCharacter::HandleManagedAttackFinished
		);
	}
	
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied)
		return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}

}

void ASlashCharacter::Jump()
{
	if (!IsAlive()) return;
	Super::Jump();
}

void ASlashCharacter::EKeyPressed()
{

	if (AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem))
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
		}
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();

		}
		else if (CanArm())
		{
			Arm();

		}

	}
}

void ASlashCharacter::Dodge()
{
	if (!IsUnoccupied() || !HasEnoughStamina())
		return;

	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		PlayMontageSection(DodgeMontage, FName("Dodge"));
		ActionState = EActionState::EAS_Dodge;

	}	
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName{ "RightHandSocket" }, this, this);
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

void ASlashCharacter::Die()
{
	ActionState = EActionState::EAS_Death;
	Super::Die();

}

void ASlashCharacter::Attack()
{

	if (CombatAttackComponent && CombatAttackComponent->IsAttackInProgress())
	{
		CombatAttackComponent->NotifyAttackInput();
		return;
	}


	if (!CanAttack()) return;

	CommitToTarget();

	if (HasAttackCommittedTarget())
	{
		SetCombatTargetActor(GetAttackCommittedTarget());
	}
	else
	{
		SetCombatTargetActor(nullptr);
	}

	CheckForFreeAttack();
	ApplyAttackFacingAssist();
	StartAttack();

}

void ASlashCharacter::CommitToTarget()
{
	if (!CombatTargetingComponent) return;

	const bool bCommitWindowStarted = CombatTargetingComponent->TryBeginAttackCommitWindow();
	if (!bCommitWindowStarted)
	{
		CombatTargetingComponent->ClearAttackCommitTarget();
		return;
	}

	CombatTargetingComponent->BeginAttackCommitTarget();
}

void ASlashCharacter::StartAttack()
{
	if (!CombatAttackComponent)
	{
		ClearAttackTarget();
		return;
	}

	if (CombatAttackComponent->TryStartAttack())
	{
		ActionState = EActionState::EAS_Attacking;
	}
	else
	{
		ClearAttackTarget();
	}
}

void ASlashCharacter::ClearAttackTarget()
{
	if (CombatTargetingComponent)
	{
		CombatTargetingComponent->ClearAttackCommitTarget();
	}
	SetCombatTargetActor(nullptr);
}

void ASlashCharacter::MontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted && (Montage == HitReactMontage || Montage == DodgeMontage)) return;

	if (Montage == HitReactMontage || Montage == EquipMontage || Montage == DodgeMontage)
	{
		ActionState = EActionState::EAS_Unoccupied;
	}

}

void ASlashCharacter::HandleManagedAttackFinished(UAnimMontage* Montage, bool bInterrupted)
{
	ClearMotionWarpTargets();

	if (bInterrupted) return;

	ActionState = EActionState::EAS_Unoccupied;
	ClearAttackTarget();
}

void ASlashCharacter::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit(ImpactPoint, Hitter);

	if (IsAlive())
	{
		ActionState = EActionState::EAS_HitReaction;
	}
	else
	{
		Die();
	}
}

bool ASlashCharacter::TryGetAttackFacingRotation(FRotator& OutRotation) const
{
	const FRotator CurrentRotation = GetActorRotation();

	if (CombatTarget)
	{
		const FVector ToTarget = CombatTarget->GetActorLocation() - GetActorLocation();
		const FVector ToTarget2D(ToTarget.X, ToTarget.Y, 0.f);

		if (ToTarget2D.IsNearlyZero())
		{
			return false;
		}

		const FRotator DesiredRotation = ToTarget2D.GetSafeNormal().Rotation();
		const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredRotation.Yaw));

		if (DeltaYaw > AttackFacingAssistMaxAngle)
		{
			return false;
		}

		OutRotation = FRotator(0.f, DesiredRotation.Yaw, 0.f);
		return true;
	}

	if (!GetController())
	{
		return false;
	}

	const FRotator ControlRotation = GetControlRotation();
	OutRotation = FRotator(0.f, ControlRotation.Yaw, 0.f);
	return true;
}

void ASlashCharacter::CheckForFreeAttack()
{
	if (CombatAttackComponent && CombatAttackComponent->ShouldUseFreeAttack())
	{
		ClearMotionWarpTargets();
		SetCombatTargetActor(nullptr);
	}
}

void ASlashCharacter::ApplyAttackFacingAssist()
{
	FRotator DesiredRotation;
	if (!TryGetAttackFacingRotation(DesiredRotation)) return;

	const FRotator CurrentRotation = GetActorRotation();
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredRotation.Yaw);

	const float ClampedStrength = FMath::Clamp(AttackFacingAssistStrength, 0.f, 1.f);
	const float AppliedYaw = CurrentRotation.Yaw + DeltaYaw * ClampedStrength;

	const FRotator NewRotation(0.f, AppliedYaw, 0.f);
	SetActorRotation(NewRotation);

}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName{ "SpineSocket" });
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName{ "RightHandSocket" });
	}
}

void ASlashCharacter::Arm()
{
	PlayMontageSection(EquipMontage, FName{ "Equip" });
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Disarm()
{
	PlayMontageSection(EquipMontage, FName{ "Unequip" });
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

bool ASlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

void ASlashCharacter::InitializeSlashOverlay(USlashOverlay* InSlashOverlay)
{
	SlashOverlay = InSlashOverlay;
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
		SlashOverlay->SetStaminaBarPercent(1.f);
		SlashOverlay->SetGold(0);
		SlashOverlay->SetSouls(0);
	}

}

bool ASlashCharacter::CanAttack() const
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}
bool ASlashCharacter::CanArm() const
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}
bool ASlashCharacter::CanDisarm() const
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}

AActor* ASlashCharacter::GetPreferredCombatTarget() const
{
	return CombatTargetingComponent ? CombatTargetingComponent->GetPreferredTarget() : nullptr;
}

bool ASlashCharacter::HasPreferredCombatTarget() const
{
	return CombatTargetingComponent && CombatTargetingComponent->HasPreferredTarget();
}

FCombatTargetInfo ASlashCharacter::GetPreferredCombatTargetInfo() const
{
	return CombatTargetingComponent ? CombatTargetingComponent->GetPreferredTargetInfo() : FCombatTargetInfo{};
}

AActor* ASlashCharacter::GetActiveCombatTarget() const
{
	return CombatTargetingComponent ? CombatTargetingComponent->GetActiveCombatTarget() : nullptr;
}

bool ASlashCharacter::HasActiveCombatTarget() const
{
	return CombatTargetingComponent && CombatTargetingComponent->HasActiveCombatTarget();
}

FCombatTargetInfo ASlashCharacter::GetActiveCombatTargetInfo() const
{
	return CombatTargetingComponent ? CombatTargetingComponent->GetActiveCombatTargetInfo() : FCombatTargetInfo{};
}

AActor* ASlashCharacter::GetAttackCommittedTarget() const
{
	return CombatTargetingComponent ? CombatTargetingComponent->GetAttackCommittedTarget() : nullptr;
}

bool ASlashCharacter::HasAttackCommittedTarget() const
{
	return CombatTargetingComponent && CombatTargetingComponent->HasAttackCommittedTarget();
}

FCombatTargetInfo ASlashCharacter::GetAttackCommittedTargetInfo() const
{
	return CombatTargetingComponent ? CombatTargetingComponent->GetAttackCommittedTargetInfo() : FCombatTargetInfo{};
}

void ASlashCharacter::ComboWindowStart() const
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->OpenComboWindow();
	}
}

void ASlashCharacter::ComboWindowStop() const
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->CloseComboWindow();
	}
}

void ASlashCharacter::ComboDecision() const
{
	if (CombatAttackComponent)
	{
		CombatAttackComponent->HandleComboDecision();
	}

}

void ASlashCharacter::AddSoul(ASoul* Soul)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSoul(Soul->GetSoul());
		SlashOverlay->SetSouls(Attributes->GetSoul());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::HealCharacter(APotion* Potion)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->Heal(Potion->GetPotionHealth());
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}
