// Copyright Epic Games, Inc. All Rights Reserved.

#include "SensorSubsystem.h"

#include "DataSource/WebSocketSensorSource.h"

namespace
{
	// TODO: move to project config (DefaultGame.ini) once we have more than one environment.
	const TCHAR* GSensorSourceUrl = TEXT("ws://127.0.0.1:8765");

	FName MakeAlarmKey(FName EquipmentId, FName SensorKey)
	{
		return FName(*FString::Printf(TEXT("%s/%s"), *EquipmentId.ToString(), *SensorKey.ToString()));
	}

	const TCHAR* ToString(ESensorAlarmSeverity Severity)
	{
		switch (Severity)
		{
		case ESensorAlarmSeverity::Warning:
			return TEXT("WARNING");
		case ESensorAlarmSeverity::Critical:
			return TEXT("CRITICAL");
		case ESensorAlarmSeverity::Normal:
		default:
			return TEXT("NORMAL");
		}
	}
}

void USensorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Thresholds.Add(TEXT("temperature"), FSensorThresholds{85.0f, 100.0f});
	Thresholds.Add(TEXT("pressure"), FSensorThresholds{45.0f, 55.0f});

	SensorDataSource = MakeShared<FWebSocketSensorSource>(GSensorSourceUrl);
	SensorDataSource->OnSensorData().AddUObject(this, &USensorSubsystem::HandleSensorData);
	SensorDataSource->Connect();
}

void USensorSubsystem::Deinitialize()
{
	if (SensorDataSource.IsValid())
	{
		SensorDataSource->Disconnect();
		SensorDataSource.Reset();
	}

	Super::Deinitialize();
}

void USensorSubsystem::HandleSensorData(FName EquipmentId, FName SensorKey, float Value, FDateTime Timestamp)
{
	UE_LOG(LogTemp, Log, TEXT("[Sensor] %s/%s = %.2f @ %s"),
		*EquipmentId.ToString(), *SensorKey.ToString(), Value, *Timestamp.ToIso8601());

	OnSensorDataReceivedDelegate.Broadcast(EquipmentId, SensorKey, Value, Timestamp);

	const ESensorAlarmSeverity NewSeverity = ComputeSeverity(SensorKey, Value);
	const FName AlarmKey = MakeAlarmKey(EquipmentId, SensorKey);
	const ESensorAlarmSeverity OldSeverity = LastSeverityByEquipmentSensor.FindRef(AlarmKey);

	if (NewSeverity != OldSeverity)
	{
		LastSeverityByEquipmentSensor.Add(AlarmKey, NewSeverity);

		UE_LOG(LogTemp, Warning, TEXT("[Alarm] %s/%s %s -> %s (%.2f)"),
			*EquipmentId.ToString(), *SensorKey.ToString(), ToString(OldSeverity), ToString(NewSeverity), Value);

		OnSensorAlarmChangedDelegate.Broadcast(EquipmentId, SensorKey, NewSeverity, OldSeverity, Value);
	}
}

ESensorAlarmSeverity USensorSubsystem::ComputeSeverity(FName SensorKey, float Value) const
{
	const FSensorThresholds* SensorThresholds = Thresholds.Find(SensorKey);
	if (!SensorThresholds)
	{
		return ESensorAlarmSeverity::Normal;
	}

	if (Value >= SensorThresholds->Critical)
	{
		return ESensorAlarmSeverity::Critical;
	}
	if (Value >= SensorThresholds->Warning)
	{
		return ESensorAlarmSeverity::Warning;
	}
	return ESensorAlarmSeverity::Normal;
}
