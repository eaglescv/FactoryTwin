// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryTwinWorldSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "UI/SensorHudWidget.h"
#include "Visualization/EquipmentActor.h"

void UFactoryTwinWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// TODO: replace this single hardcoded spawn with a config-driven equipment list
	// once more than EQ-01 exists.
	AEquipmentActor* Equipment = InWorld.SpawnActorDeferred<AEquipmentActor>(
		AEquipmentActor::StaticClass(), FTransform(FVector::ZeroVector));

	if (Equipment)
	{
		Equipment->EquipmentId = TEXT("EQ-01");
		Equipment->FinishSpawning(FTransform(FVector::ZeroVector));
	}

	// No dedicated-server rendering, so skip HUD setup there.
	if (InWorld.GetNetMode() != NM_DedicatedServer)
	{
		if (USensorHudWidget* Hud = CreateWidget<USensorHudWidget>(&InWorld, USensorHudWidget::StaticClass()))
		{
			Hud->EquipmentId = TEXT("EQ-01");
			Hud->AddToViewport();
		}
	}
}

bool UFactoryTwinWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
