#pragma once


#include "CoreMinimal.h"

void WDrawSphere(const UWorld* InWorld, const FVector& Center, float Radius = 25.f, int32 Segments = 12, const FColor& Color = FColor::Yellow, bool PersistentLines = false, float LifeTime = 5.f);
void WDrawLine(const UWorld* InWorld, const FVector& StartLine, const FVector& EndLine, bool PersistentLines = false, const FColor& Color = FColor::Red, float LifeTime = 5.f);
void WDrawPoint(const UWorld* InWorld, const FVector& Position, bool PersistentLines = false, float size = 15.f, const FColor& PColor = FColor::Red, float LifeTime = 5.f);
void DrawVector(const UWorld* InWorld, const FVector& StartLine, const FVector& EndLine, bool PersistentLines = false);
void WDrawBox(const UWorld* InWorld, const FVector& Center, bool PersistentLines = true);