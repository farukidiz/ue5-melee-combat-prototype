#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

AWeapon::AWeapon()
{

	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::BoxOverlapBegin);
}

void AWeapon::Equip(USceneComponent* InParent, FName SocketName, AActor* NewOwner, APawn* NewInstigator)
{
	ItemState = EItemState::EIS_Equipped;
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, SocketName);
	DisableSphereCollision();
	if(Cast<ASlashCharacter>(NewOwner) != nullptr)
	PlayEquipSound();
	DeactivateEmbers();
}

void AWeapon::DeactivateEmbers()
{
	if (ItemEffect)
	{
		ItemEffect->Deactivate();
	}
}

void AWeapon::DisableSphereCollision()
{
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::PlayEquipSound()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& SocketName)
{
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, AttachmentRules, SocketName);
}

void AWeapon::ReInitWeaponIgnoreActors()
{
	IgnoreActors.Empty();
	IgnoreActors.Add(this);
	IgnoreActors.Add(GetOwner());
}
void AWeapon::BeginAttackWindow()
{
	ReInitWeaponIgnoreActors();
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::EndAttackWindow()
{
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsActorSameType(OtherActor)) return;
	FHitResult BoxHit;
	BoxTrace(BoxHit);

	if (BoxHit.GetActor() && !IsActorSameType(BoxHit.GetActor()))
	{
		UGameplayStatics::ApplyDamage(BoxHit.GetActor(),Damage,GetInstigator()->GetController(),this,UDamageType::StaticClass());
		ExecuteGetHit(BoxHit);
	}
}

bool AWeapon::IsActorSameType(AActor* OtherActor)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OtherActor) return false;

	return OwnerActor->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
	if (HitInterface)
	{
		HitInterface->GetHit(BoxHit.ImpactPoint, GetOwner());
	}
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	
	FVector StartTraceLocation = BoxTraceStart->GetComponentLocation();
	FVector EndTraceLocation = BoxTraceEnd->GetComponentLocation();
	UKismetSystemLibrary::BoxTraceSingle(this, StartTraceLocation, EndTraceLocation, BoxTraceExtent, BoxTraceStart->GetComponentRotation(),
										ETraceTypeQuery::TraceTypeQuery1, false, IgnoreActors, (bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None), BoxHit, true);
	if (AActor* HitActor = BoxHit.GetActor())
	{
		IgnoreActors.AddUnique(HitActor);
	}

}
