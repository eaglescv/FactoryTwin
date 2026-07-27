// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "ISensorDataSource.h"

/**
 * Stand-in for a future real OPC UA client. Exists to prove that
 * ISensorDataSource consumers (USensorSubsystem, AEquipmentActor,
 * USensorHudWidget) genuinely don't care which transport feeds them --
 * this generates its own readings internally instead of talking to a real
 * OPC UA server, but from the outside it behaves exactly like
 * FWebSocketSensorSource (same interface, same delegate).
 *
 * TODO: replace the internal simulation with a real OPC UA client (e.g. the
 * open62541 library) once there's an actual OPC UA server endpoint to talk to.
 * EndpointUrl is already plumbed through for that (e.g. "opc.tcp://host:4840").
 */
class FACTORYTWIN_API FMockOpcUaSensorSource : public ISensorDataSource
{
public:
	explicit FMockOpcUaSensorSource(const FString& InEndpointUrl);
	virtual ~FMockOpcUaSensorSource() override;

	// ISensorDataSource
	virtual void Connect() override;
	virtual void Disconnect() override;
	virtual bool IsConnected() const override;
	virtual FOnSensorData& OnSensorData() override { return SensorDataDelegate; }

private:
	bool Tick(float DeltaTime);

	FString EndpointUrl;
	FOnSensorData SensorDataDelegate;
	FTSTicker::FDelegateHandle TickerHandle;
	bool bConnected = false;

	float TemperatureValue = 70.0f;
	float PressureValue = 30.0f;
};
