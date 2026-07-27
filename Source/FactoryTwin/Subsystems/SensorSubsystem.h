// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SensorSubsystem.generated.h"

class ISensorDataSource;

/**
 * Game-instance-level entry point for live sensor data. Owns the current
 * ISensorDataSource (WebSocket today, OPC UA later) and logs every reading.
 * Later slices will fan OnSensorData out to 3D actors / UI instead of just logging.
 */
UCLASS()
class FACTORYTWIN_API USensorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void HandleSensorData(FName EquipmentId, FName SensorKey, float Value, FDateTime Timestamp);

	TSharedPtr<ISensorDataSource> SensorDataSource;
};
