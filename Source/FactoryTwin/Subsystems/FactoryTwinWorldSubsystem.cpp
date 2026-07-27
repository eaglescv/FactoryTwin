// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryTwinWorldSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "SensorSubsystem.h"
#include "UI/SensorHudWidget.h"
#include "Visualization/EquipmentActor.h"

namespace
{
	// Spacing between spawned equipment actors / stacked HUD lines when there's more than one.
	constexpr float EquipmentSpacingX = 300.0f;
	constexpr float HudLineSpacingY = 90.0f;
}

void UFactoryTwinWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	TArray<FString> EquipmentIds = {TEXT("EQ-01")};
	if (UGameInstance* GameInstance = InWorld.GetGameInstance())
	{
		if (USensorSubsystem* SensorSubsystem = GameInstance->GetSubsystem<USensorSubsystem>())
		{
			EquipmentIds = SensorSubsystem->KnownEquipmentIds;
		}
	}

	for (int32 Index = 0; Index < EquipmentIds.Num(); ++Index)
	{
		const FName EquipmentId(*EquipmentIds[Index]);
		const FTransform SpawnTransform(FVector(Index * EquipmentSpacingX, 0.0f, 0.0f));

		AEquipmentActor* Equipment = InWorld.SpawnActorDeferred<AEquipmentActor>(AEquipmentActor::StaticClass(), SpawnTransform);
		if (Equipment)
		{
			Equipment->EquipmentId = EquipmentId;
			Equipment->FinishSpawning(SpawnTransform);
		}
	}

	if (InWorld.GetNetMode() != NM_DedicatedServer)
	{
		// The game viewport subsystem isn't guaranteed to be ready this early in world
		// begin play, so AddToViewport() can silently no-op here. Defer HUD creation by
		// one tick so it runs once the viewport is definitely up.
		TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
			[WeakWorld, EquipmentIds](float /*DeltaTime*/) -> bool
			{
				UWorld* World = WeakWorld.Get();
				if (!World)
				{
					return false;
				}

				for (int32 Index = 0; Index < EquipmentIds.Num(); ++Index)
				{
					const FName EquipmentId(*EquipmentIds[Index]);
					if (USensorHudWidget* Hud = CreateWidget<USensorHudWidget>(World, USensorHudWidget::StaticClass()))
					{
						Hud->EquipmentId = EquipmentId;
						Hud->ScreenOffsetY = Index * HudLineSpacingY;
						Hud->AddToViewport();
					}
				}
				return false; // one-shot
			}),
			0.0f);
	}
}

bool UFactoryTwinWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
