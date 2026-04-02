#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items.generated.h"


class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

enum class EItemState : uint8{
	EIS_Hovering,
	EIS_Equipped,
};

UCLASS()
class SLASHLAST_API AItems : public AActor
{
	GENERATED_BODY()
	
public:	

	AItems();


	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	void SpawnPickupEffect();
	void SpawnPickupSound();

	UFUNCTION()
	virtual void SphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameter")
	float Amplitude{ 0.25f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameter")
	float TimeConstant{ 5.f };

	UFUNCTION(BlueprintPure)
	float TransformedSin();

	UFUNCTION(BlueprintPure)
	float TransformedCos();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Sphere;

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* ItemEffect;

	EItemState ItemState = EItemState::EIS_Hovering;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffect;

	UPROPERTY(EditAnywhere)
	USoundBase* PickupSound;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunningTime{};

};
