// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Broadcast whenever a sensor reading arrives from the underlying data source,
 * regardless of transport (WebSocket today, OPC UA later).
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnSensorData, FName /*EquipmentId*/, FName /*SensorKey*/, float /*Value*/, FDateTime /*Timestamp*/);

/**
 * Transport-agnostic source of live sensor readings. Concrete implementations
 * (WebSocket, OPC UA, ...) plug in behind this interface so consumers such as
 * USensorSubsystem never depend on a specific protocol.
 */
class FACTORYTWIN_API ISensorDataSource
{
public:
	virtual ~ISensorDataSource() = default;

	virtual void Connect() = 0;
	virtual void Disconnect() = 0;
	virtual bool IsConnected() const = 0;

	virtual FOnSensorData& OnSensorData() = 0;
};
