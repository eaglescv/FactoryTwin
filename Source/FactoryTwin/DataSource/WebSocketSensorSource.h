// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "ISensorDataSource.h"

class IWebSocket;

/**
 * ISensorDataSource implementation backed by a single WebSocket connection
 * to an external simulator (or, eventually, an OPC UA-to-WebSocket bridge).
 * Expects newline-delimited JSON messages shaped like:
 *   {"equipmentId":"EQ-01","sensorKey":"temperature","value":72.5,"ts":"2026-07-27T14:03:00Z"}
 */
class FACTORYTWIN_API FWebSocketSensorSource : public ISensorDataSource
{
public:
	explicit FWebSocketSensorSource(const FString& InServerUrl);
	virtual ~FWebSocketSensorSource() override;

	// ISensorDataSource
	virtual void Connect() override;
	virtual void Disconnect() override;
	virtual bool IsConnected() const override;
	virtual FOnSensorData& OnSensorData() override { return SensorDataDelegate; }

private:
	void HandleConnected();
	void HandleConnectionError(const FString& Error);
	void HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void HandleMessage(const FString& Message);

	void ScheduleReconnect();
	void ClearReconnectTimer();

	FString ServerUrl;
	TSharedPtr<IWebSocket> WebSocket;
	FOnSensorData SensorDataDelegate;
	FTSTicker::FDelegateHandle ReconnectTickerHandle;

	static constexpr float ReconnectDelaySeconds = 3.0f;
};
