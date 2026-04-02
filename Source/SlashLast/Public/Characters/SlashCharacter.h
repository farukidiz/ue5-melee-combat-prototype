#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class UCombatTargetingComponent;
class UCombatAttackComponent;
class UAnimInstance;
class USlashOverlay;
class AItems;
class ATreasure;
class ASoul;

struct FCombatTargetInfo;
struct FInputActionValue;

enum class ECharacterState : uint8;
enum class EActionState : uint8;


UCLASS()
class SLASHLAST_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void InitializeSlashOverlay(USlashOverlay* InSlashOverlay);
	void Tick(float DeltaTime) override;


	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;

	void AttachWeaponToHand();
	void AttachWeaponToBack();

	AActor* GetActiveCombatTarget() const;
	bool HasActiveCombatTarget() const;
	FCombatTargetInfo GetActiveCombatTargetInfo() const;

	AActor* GetAttackCommittedTarget() const;
	bool HasAttackCommittedTarget() const;
	FCombatTargetInfo GetAttackCommittedTargetInfo() const;

	void ComboWindowStart() const;
	void ComboWindowStop() const;
	void ComboDecision() const;

	virtual void AddSoul(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;
	virtual void HealCharacter(class APotion* Potion) override;
	FORCEINLINE virtual void SetOverlappingItem(AItems* Item) override { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState()const { return CharacterState; }
	FORCEINLINE EActionState GetActionState()const { return ActionState; }
	FORCEINLINE AWeapon* GetEquippedWeapon()const { return EquippedWeapon; }
	FORCEINLINE UCombatTargetingComponent* GetCombatTargetingComponent() const { return CombatTargetingComponent; }

protected:

	virtual void BeginPlay() override;
	virtual bool CanAttack() const override;
	virtual void Jump() override;
	virtual void Attack() override;

	void CommitToTarget();
	void StartAttack();

	void ClearAttackTarget();

	// Input callback functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EKeyPressed();
	void Dodge();

	void EquipWeapon(AWeapon* Weapon);
	virtual void Die() override;
	virtual void MontageEnd(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void HandleDamage(float DamageAmount) override;
	void HandleManagedAttackFinished(UAnimMontage* Montage, bool bInterrupted);
	
	AActor* GetPreferredCombatTarget() const;
	bool HasPreferredCombatTarget() const;
	FCombatTargetInfo GetPreferredCombatTargetInfo() const;

	bool TryGetAttackFacingRotation(FRotator& OutRotation) const;
	void CheckForFreeAttack();
	void ApplyAttackFacingAssist();

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* DodgeAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	UCombatTargetingComponent* CombatTargetingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	UCombatAttackComponent* CombatAttackComponent;

	UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
	bool bEnableAttackFacingAssist = true;

	UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
	float AttackFacingAssistMaxDistance = 500.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
	float AttackFacingAssistMaxAngle = 100.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
	float AttackFacingAssistStrength = 0.6f;

private:

	bool CanArm() const;
	bool CanDisarm() const;
	void Arm();
	void Disarm();
	bool IsUnoccupied();
	bool HasEnoughStamina();

	UPROPERTY(VisibleAnyWhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnyWhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnyWhere, Category = Hair)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnyWhere, Category = Hair)
	UGroomComponent* EyeBrows;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DodgeMontage;

	UPROPERTY(VisibleInstanceOnly)
	AItems* OverlappingItem = nullptr;

	UPROPERTY(VisibleInstanceOnly)
	USlashOverlay* SlashOverlay;

	EActionState ActionState;

	ECharacterState CharacterState;


};
