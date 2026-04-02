#pragma once


UENUM(BlueprintType)
enum class EActionState : uint8 {

	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_HitReaction UMETA(DisplayName = "HitReaction"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_EquippingWeapon UMETA(DisplayName = "Equipping Weapon"),
	EAS_Dodge UMETA(DisplayName = "Dodge"),
	EAS_Death UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class ECharacterState : uint8 {
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two-Handed Weapon")
};

UENUM(BlueprintType)
enum class EDeathPose : uint8 {

	EDP_DeathPoseBack1 UMETA(DisplayName = "DeathPoseBack1"),
	EDP_DeathPoseBack2 UMETA(DisplayName = "DeathPoseBack2"),
	EDP_DeathPoseBack3 UMETA(DisplayName = "DeathPoseBack3"),
	EDP_DeathPoseFront1 UMETA(DisplayName = "DeathPoseFront1"),
	EDP_DeathPoseFront2 UMETA(DisplayName = "DeathPoseFront2"),
	EDP_DeathPoseFront3 UMETA(DisplayName = "DeathPoseFront3")

};

UENUM(BlueprintType)
enum class EEnemyState : uint8 {

	EES_NoState UMETA(DisplayName = "NoState"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_HitReact UMETA(DisplayName = "HitReact"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_Engaged UMETA(DisplayName = "Engaged")

};