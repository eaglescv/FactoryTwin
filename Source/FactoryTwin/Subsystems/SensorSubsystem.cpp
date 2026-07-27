// Copyright Epic Games, Inc. All Rights Reserved.

#include "SensorSubsystem.h"

#include "DataSource/WebSocketSensorSource.h"

namespace
{
	// TODO: move to project config (DefaultGame.ini) once we have more than one environment.
	const TCHAR* GSensorSourceUrl = TEXT("ws://127.0.0.1:8765");
}

void USensorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

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
}
