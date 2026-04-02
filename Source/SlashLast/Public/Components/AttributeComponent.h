#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASHLAST_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	virtual void BeginPlay() override;



private:
	
	UPROPERTY(EditAnywhere, Category = "Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float Stamina;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float DodgeCost = 17.f;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float RegenRate = 2.f;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	int32 Gold{};

	UPROPERTY(EditAnywhere, Category = "Attributes")
	int32 Soul{};

public:
	void ReceiveDamage(float Damage);
	void Heal(float AmountOfHeal);
	void UseStamina(float StaminaCost);
	float GetHealthPercent() const;
	float GetStaminaPercent() const;
	void AddGold(int32 AmountOfGold) { Gold += AmountOfGold; }
	void AddSoul(int32 AmountOfSoul) { Soul += AmountOfSoul; }
	void RegenStamina(float DeltaTime);
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSoul() const { return Soul; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE bool IsAlive() const { return Health > 0.f; }
	FORCEINLINE bool IsStaminaNotFull() const { return Stamina < MaxStamina ; }
};
