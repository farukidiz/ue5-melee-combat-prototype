#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Characters/SlashCharacter.h"

void ASlashHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* Controller = GetOwningPlayerController(); 
	if (!Controller || !SlashOverlayClass) return;

	SlashOverlay = CreateWidget<USlashOverlay>(Controller, SlashOverlayClass);

	if (!SlashOverlay) return;
	SlashOverlay->AddToViewport();

	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(Controller->GetPawn()))
	{
		SlashCharacter->InitializeSlashOverlay(SlashOverlay);
	}

}
