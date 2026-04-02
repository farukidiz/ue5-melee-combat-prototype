#include "Enemy/Enemy.h"
#include "Components/MeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Characters/CharacterTypes.h"
#include "Enemy/EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Items/Weapons/Weapon.h"
#include "BrainComponent.h"
#include "Items/Soul.h"
#include "Items/Potion.h"


AEnemy::AEnemy() : LoseInterestRadius(LoseSightRadius * 0.95f), EnemyState(EEnemyState::EES_Patrolling)
{

	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);


	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HeathBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	HealthBarWidget->GetUserWidgetObject();

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitializeEnemy();
	SetSightConfig();
	if (PerceptionComp)
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemy::OnTargetPerceptionUpdated);
	}

}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AEnemyAIController>(GetController());
	HideHealthBar();
	SpawnDefaultWeapon();
	Tags.Add(FName{ "Enemy" });
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		if (DefaultWeapon)
		{
			DefaultWeapon->Equip(GetMesh(), FName{ "WeaponSocket" }, this, this);
			EquippedWeapon = DefaultWeapon;

		}
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{

	APawn* SeenPawn = Cast<APawn>(Actor);
	if (!SeenPawn) return;

	const bool bIsSight = (Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()));
	if (!bIsSight) return;

	if (!SeenPawn->ActorHasTag(FName{ "EngageableTarget" })) return;


	if (Stimulus.WasSuccessfullySensed() && EnemyController) 
	{ 
		EnemyController->HandleTargetSeen(SeenPawn); 
	}
	else
	{ 
		EnemyController->HandleTargetLost(SeenPawn); 
	}

}

void AEnemy::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{
	ShowHealthBar();
	Super::GetHit(ImpactPoint, Hitter);

	if (IsAlive())
	{
		SetEnemyHitReactState();
	}
	else
	{
		EnemyController->ClearAttackTimer();
		Die();
	}
}

void AEnemy::SetEnemyHitReactState()
{
	if (!EnemyController) return;

	EnemyController->ClearAttackTimer();
	EnemyController->StopMovement();

	EnemyState = EEnemyState::EES_HitReact;
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	HandleDamage(DamageAmount);
	if (EventInstigator && EnemyController)
	{
		CombatTarget = EventInstigator->GetPawn();
	}

	return DamageAmount;
}


void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (HealthBarWidget && Attributes)
	{
		HealthBarWidget->SetHealthPercentage(Attributes->GetHealthPercent());
	}

}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

bool AEnemy::IsOutsideLoseInterestRadius() const
{
	return !IsTargetInRange(CombatTarget, LoseInterestRadius);
}

bool AEnemy::IsOutsideLoseSightRadius() const
{
	return !IsTargetInRange(CombatTarget, LoseSightRadius);
}

bool AEnemy::IsOutsideAttackRadius() const
{
	return !IsTargetInRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius() const
{
	return IsTargetInRange(CombatTarget, AttackRadius);
}


bool AEnemy::IsChasing() const
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking() const
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsPatrolling() const
{
	return EnemyState == EEnemyState::EES_Patrolling;
}

bool AEnemy::IsDead() const
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged() const
{
	return EnemyState == EEnemyState::EES_Engaged;
}

bool AEnemy::IsHitReact() const
{
	return EnemyState == EEnemyState::EES_HitReact;
}

bool AEnemy::CanAttack() const
{
	return IsInsideAttackRadius() && !IsEngaged() && !IsDead() && !IsHitReact();
}

bool AEnemy::IsTargetInRange(AActor* Target, double Radius) const
{
	if (!IsValid(Target)) return false;

	return (Target->GetActorLocation() - GetActorLocation()).Size() <= Radius;

}

void AEnemy::Attack()
{
	PlayAttackMontage();
	EnemyState = EEnemyState::EES_Engaged;
}

void AEnemy::MontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	Super::MontageEnd(Montage, bInterrupted);
	if (IsDead() || bInterrupted || !EnemyController) return;

	if (Montage == AttackMontage || Montage == HitReactMontage)
	{
		EnemyState = EEnemyState::EES_NoState;
		EnemyController->EvaluateStateFromContext();
	}
}

AActor* AEnemy::ChooseNewPatrolTarget()
{
	if (PatrolTargets.Num() <= 1) return nullptr;

	int32 NewIndex;

	do
	{
		NewIndex = FMath::RandRange(0, PatrolTargets.Num() - 1);

	} while (PatrolTargets[NewIndex] == PatrolTarget);

	return PatrolTarget = PatrolTargets[NewIndex];

}

void AEnemy::WalkPatrollingSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
}

void AEnemy::WalkChasingSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
}

void AEnemy::Die()
{
	if (IsDead()) return;
	EnemyState = EEnemyState::EES_Dead;
	Super::Die();
	SetLifeSpan(DeathLifeSpan);
	DisableAIMovement();
	HideHealthBar();
	SpawnPickupItem();
}

void AEnemy::SpawnPickupItem()
{
	UWorld* World = GetWorld();
	int32 RandomValue = FMath::FRandRange(0.f, PotionWeight + SoulWeight);
	if (RandomValue <= PotionWeight)
	{
		SpawnPotion();
	}
	else
	{
		SpawnSoul();
	}

}

void AEnemy::SpawnPotion()
{
	UWorld* World = GetWorld();

	if (World && PotionClass && Attributes)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 125.f);
		APotion* SpawnedPotion = World->SpawnActor<APotion>(PotionClass, SpawnLocation, GetActorRotation());
		if (SpawnedPotion)
		{
			SpawnedPotion->SetOwner(this);
		}
	}
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();

	if (World && SoulClass && Attributes)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 125.f);
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, GetActorRotation());
		if (SpawnedSoul)
		{
			SpawnedSoul->SetSoul(Attributes->GetSoul());
			SpawnedSoul->SetOwner(this);
		}
	}

}

void AEnemy::DisableAIMovement()
{
	if (EnemyController)
	{
		EnemyController->StopMovement();
		EnemyController->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void AEnemy::Destroyed()
{
	Super::Destroyed();

	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}
void AEnemy::SetSightConfig()
{
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	SightConfig->SetMaxAge(1.f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	PerceptionComp->RequestStimuliListenerUpdate();
}

bool AEnemy::CanBeTargeted() const
{
	return !IsDead();
}

bool AEnemy::IsDeadForTargeting() const
{
	return IsDead();
}

FVector AEnemy::GetCombatTargetPoint() const
{
	return GetActorLocation() + FVector(0.f, 0.f, 40.f);
}