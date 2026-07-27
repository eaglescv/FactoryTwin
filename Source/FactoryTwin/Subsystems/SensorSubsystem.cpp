// Copyright Epic Games, Inc. All Rights Reserved.

#include "SensorSubsystem.h"

#include "DataSource/WebSocketSensorSource.h"

namespace
{
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

FLinearColor GetSensorAlarmSeverityColor(ESensorAlarmSeverity Severity)
{
	switch (Severity)
	{
	case ESensorAlarmSeverity::Critical:
		return FLinearColor::Red;
	case ESensorAlarmSeverity::Warning:
		return FLinearColor(1.0f, 0.65f, 0.0f); // amber
	case ESensorAlarmSeverity::Normal:
	default:
		return FLinearColor::Green;
	}
}

void USensorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (KnownEquipmentIds.Num() == 0)
	{
		KnownEquipmentIds.Add(TEXT("EQ-01"));
	}

	Thresholds.Add(TEXT("temperature"), FSensorThresholds{85.0f, 100.0f});
	Thresholds.Add(TEXT("pressure"), FSensorThresholds{45.0f, 55.0f});

	SensorDataSource = MakeShared<FWebSocketSensorSource>(ServerUrl);
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
	// Verbose (suppressed by default) so this doesn't drown out the widget/rendering
	// debug logs while tracking down the HUD-not-rendering issue.
	UE_LOG(LogTemp, Verbose, TEXT("[Sensor] %s/%s = %.2f @ %s"),
		*EquipmentId.ToString(), *SensorKey.ToString(), Value, *Timestamp.ToIso8601());

	OnSensorDataReceivedDelegate.Broadcast(EquipmentId, SensorKey, Value, Timestamp);

	const ESensorAlarmSeverity NewSeverity = GetSeverity(SensorKey, Value);
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

ESensorAlarmSeverity USensorSubsystem::GetSeverity(FName SensorKey, float Value) const
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
