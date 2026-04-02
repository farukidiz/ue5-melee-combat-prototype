#include "DebugDrawUtils.h"
#include "DrawDebugHelpers.h"

void WDrawSphere(const UWorld* InWorld, const FVector& Center, float Radius, int32 Segments, const FColor& Color, bool PersistentLines, float LifeTime)
{
	if (InWorld)
	{
		DrawDebugSphere(InWorld, Center, Radius, Segments, FColor::Red, PersistentLines, LifeTime);
	}
}

void WDrawLine(const UWorld* InWorld, const FVector& StartLine, const FVector& EndLine, bool PersistentLines, const FColor& Color, float LifeTime)
{
	if (InWorld)
	{
		DrawDebugLine(InWorld, StartLine, EndLine, Color, PersistentLines, LifeTime);
	}
}

void WDrawPoint(const UWorld* InWorld, const FVector& Position, bool PersistentLines, float size, const FColor& PColor, float LifeTime)
{
	if (InWorld)
	{
		DrawDebugPoint(InWorld, Position, size, PColor, PersistentLines, LifeTime);
	}
}

void DrawVector(const UWorld* InWorld, const FVector& StartLine, const FVector& EndLine, bool PersistentLines)
{
	WDrawLine(InWorld, StartLine, EndLine, PersistentLines);
	WDrawPoint(InWorld, EndLine, PersistentLines);

}
void WDrawBox(const UWorld* InWorld, const FVector& Center, bool PersistentLines)
{
	if (InWorld)
	{
		DrawDebugBox(InWorld, Center, FVector{50.f, 50.f, 50.f}, FColor::Red, PersistentLines, -1.f, 0U, 1.f);
	}
}

