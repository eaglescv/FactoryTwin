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

// Which ISensorDataSource implementation USensorSubsystem should construct. OpcUaMock proves the
// ISensorDataSource abstraction is genuinely swappable ahead of a real OPC UA client existing.
UENUM(BlueprintType)
enum class ESensorDataSourceType : uint8
{
	WebSocket,
	OpcUaMock
};

/**
 * Fired only when a (EquipmentId, SensorKey) pair's severity actually changes
 * (e.g. Normal -> Warning), not on every reading — so this is safe to hook up
 * to logging, notifications, etc. without spamming on every tick.
 */
DECLARE_MULTICAST_DELEGATE_FiveParams(FOnSensorAlarmChanged, FName /*EquipmentId*/, FName /*SensorKey*/, ESensorAlarmSeverity /*NewSeverity*/, ESensorAlarmSeverity /*OldSeverity*/, float /*Value*/);

/** Shared Normal/Warning/Critical -> color mapping so actors, HUD, etc. never redefine it themselves. */
FACTORYTWIN_API FLinearColor GetSensorAlarmSeverityColor(ESensorAlarmSeverity Severity);

/**
 * Game-instance-level entry point for live sensor data. Owns the current
 * ISensorDataSource (WebSocket or a mock OPC UA source, selected via
 * DataSourceType), logs every reading, and
 * re-broadcasts it via OnSensorDataReceived so 3D actors / UI can subscribe
 * without knowing anything about the underlying transport. Also owns the
 * per-sensor alarm thresholds (the single source of truth other classes
 * should query via GetSeverity() rather than hardcoding their own copies)
 * and broadcasts OnSensorAlarmChanged on severity transitions.
 */
UCLASS(Config = Game)
class FACTORYTWIN_API USensorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FOnSensorData& OnSensorDataReceived() { return OnSensorDataReceivedDelegate; }
	FOnSensorAlarmChanged& OnSensorAlarmChanged() { return OnSensorAlarmChangedDelegate; }

	ESensorAlarmSeverity GetSeverity(FName SensorKey, float Value) const;

	// Which ISensorDataSource implementation to construct in Initialize(). Set via
	// Config/DefaultGame.ini, section [/Script/FactoryTwin.SensorSubsystem].
	UPROPERTY(Config)
	ESensorDataSourceType DataSourceType = ESensorDataSourceType::WebSocket;

	// Connection endpoint for the active data source (ws:// URL for WebSocket,
	// opc.tcp:// URL once a real OPC UA client replaces the mock). Set via
	// Config/DefaultGame.ini, section [/Script/FactoryTwin.SensorSubsystem].
	UPROPERTY(Config)
	FString ServerUrl = TEXT("ws://127.0.0.1:8765");

	// Equipment to spawn actors/HUD for at world begin play. Populated from
	// Config/DefaultGame.ini (+KnownEquipmentIds=... entries); add more there
	// to bring another piece of equipment online without touching code.
	UPROPERTY(Config)
	TArray<FString> KnownEquipmentIds;

private:
	struct FSensorThresholds
	{
		float Warning = 0.0f;
		float Critical = 0.0f;
	};

	void HandleSensorData(FName EquipmentId, FName SensorKey, float Value, FDateTime Timestamp);

	TSharedPtr<ISensorDataSource> SensorDataSource;
	FOnSensorData OnSensorDataReceivedDelegate;
	FOnSensorAlarmChanged OnSensorAlarmChangedDelegate;

	TMap<FName, FSensorThresholds> Thresholds;
	TMap<FName, ESensorAlarmSeverity> LastSeverityByEquipmentSensor;
};
