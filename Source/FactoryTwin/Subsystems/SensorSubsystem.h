// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataSource/ISensorDataSource.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SensorSubsystem.generated.h"

/**
 * Game-instance-level entry point for live sensor data. Owns the current
 * ISensorDataSource (WebSocket today, OPC UA later), logs every reading, and
 * re-broadcasts it via OnSensorDataReceived so 3D actors / UI can subscribe
 * without knowing anything about the underlying transport.
 */
UCLASS()
class FACTORYTWIN_API USensorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FOnSensorData& OnSensorDataReceived() { return OnSensorDataReceivedDelegate; }

private:
	void HandleSensorData(FName EquipmentId, FName SensorKey, float Value, FDateTime Timestamp);

	TSharedPtr<ISensorDataSource> SensorDataSource;
	FOnSensorData OnSensorDataReceivedDelegate;
};
