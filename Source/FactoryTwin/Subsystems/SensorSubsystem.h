// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataSource/ISensorDataSource.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SensorSubsystem.generated.h"

UENUM(BlueprintType)
enum class ESensorAlarmSeverity : uint8
{
	Normal,
	Warning,
	Critical
};

/**
 * Fired only when a (EquipmentId, SensorKey) pair's severity actually changes
 * (e.g. Normal -> Warning), not on every reading — so this is safe to hook up
 * to logging, notifications, etc. without spamming on every tick.
 */
DECLARE_MULTICAST_DELEGATE_FiveParams(FOnSensorAlarmChanged, FName /*EquipmentId*/, FName /*SensorKey*/, ESensorAlarmSeverity /*NewSeverity*/, ESensorAlarmSeverity /*OldSeverity*/, float /*Value*/);

/**
 * Game-instance-level entry point for live sensor data. Owns the current
 * ISensorDataSource (WebSocket today, OPC UA later), logs every reading, and
 * re-broadcasts it via OnSensorDataReceived so 3D actors / UI can subscribe
 * without knowing anything about the underlying transport. Also tracks
 * per-sensor alarm severity and broadcasts OnSensorAlarmChanged on transitions.
 */
UCLASS()
class FACTORYTWIN_API USensorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FOnSensorData& OnSensorDataReceived() { return OnSensorDataReceivedDelegate; }
	FOnSensorAlarmChanged& OnSensorAlarmChanged() { return OnSensorAlarmChangedDelegate; }

private:
	struct FSensorThresholds
	{
		float Warning = 0.0f;
		float Critical = 0.0f;
	};

	void HandleSensorData(FName EquipmentId, FName SensorKey, float Value, FDateTime Timestamp);
	ESensorAlarmSeverity ComputeSeverity(FName SensorKey, float Value) const;

	TSharedPtr<ISensorDataSource> SensorDataSource;
	FOnSensorData OnSensorDataReceivedDelegate;
	FOnSensorAlarmChanged OnSensorAlarmChangedDelegate;

	// TODO: consolidate with the WarningTemperature/CriticalTemperature constants duplicated in
	// AEquipmentActor and USensorHudWidget once there's a shared config source (see WORKLOG structure-cleanup notes).
	TMap<FName, FSensorThresholds> Thresholds;
	TMap<FName, ESensorAlarmSeverity> LastSeverityByEquipmentSensor;
};
