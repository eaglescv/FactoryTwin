// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryTwinWorldSubsystem.generated.h"

/**
 * Spawns the known equipment actors as soon as the world begins play, regardless
 * of which map or GameMode is active. Keeps the visualization slice reproducible
 * from source control alone, with no manual level-editing step required.
 */
UCLASS()
class FACTORYTWIN_API UFactoryTwinWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
