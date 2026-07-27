// Copyright Epic Games, Inc. All Rights Reserved.

#include "WebSocketSensorSource.h"

#include "Dom/JsonObject.h"
#include "IWebSocket.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "WebSocketsModule.h"

FWebSocketSensorSource::FWebSocketSensorSource(const FString& InServerUrl)
	: ServerUrl(InServerUrl)
{
}

FWebSocketSensorSource::~FWebSocketSensorSource()
{
	Disconnect();
}

void FWebSocketSensorSource::Connect()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		return;
	}

	ClearReconnectTimer();

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerUrl);

	WebSocket->OnConnected().AddRaw(this, &FWebSocketSensorSource::HandleConnected);
	WebSocket->OnConnectionError().AddRaw(this, &FWebSocketSensorSource::HandleConnectionError);
	WebSocket->OnClosed().AddRaw(this, &FWebSocketSensorSource::HandleClosed);
	WebSocket->OnMessage().AddRaw(this, &FWebSocketSensorSource::HandleMessage);

	WebSocket->Connect();
}

void FWebSocketSensorSource::Disconnect()
{
	ClearReconnectTimer();

	if (WebSocket.IsValid())
	{
		WebSocket->OnConnected().RemoveAll(this);
		WebSocket->OnConnectionError().RemoveAll(this);
		WebSocket->OnClosed().RemoveAll(this);
		WebSocket->OnMessage().RemoveAll(this);

		if (WebSocket->IsConnected())
		{
			WebSocket->Close();
		}

		WebSocket.Reset();
	}
}

bool FWebSocketSensorSource::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

void FWebSocketSensorSource::HandleConnected()
{
	UE_LOG(LogTemp, Log, TEXT("[SensorSource] Connected to %s"), *ServerUrl);
}

void FWebSocketSensorSource::HandleConnectionError(const FString& Error)
{
	UE_LOG(LogTemp, Warning, TEXT("[SensorSource] Connection error on %s: %s"), *ServerUrl, *Error);
	ScheduleReconnect();
}

void FWebSocketSensorSource::HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Warning, TEXT("[SensorSource] Connection closed (Code=%d, Clean=%s): %s"),
		StatusCode, bWasClean ? TEXT("true") : TEXT("false"), *Reason);
	ScheduleReconnect();
}

void FWebSocketSensorSource::HandleMessage(const FString& Message)
{
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Message);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SensorSource] Failed to parse JSON message: %s"), *Message);
		return;
	}

	FString EquipmentIdStr;
	FString SensorKeyStr;
	double Value = 0.0;
	FString TimestampStr;

	if (!JsonObject->TryGetStringField(TEXT("equipmentId"), EquipmentIdStr) ||
		!JsonObject->TryGetStringField(TEXT("sensorKey"), SensorKeyStr) ||
		!JsonObject->TryGetNumberField(TEXT("value"), Value) ||
		!JsonObject->TryGetStringField(TEXT("ts"), TimestampStr))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SensorSource] Malformed sensor payload: %s"), *Message);
		return;
	}

	FDateTime Timestamp;
	if (!FDateTime::ParseIso8601(*TimestampStr, Timestamp))
	{
		Timestamp = FDateTime::UtcNow();
	}

	SensorDataDelegate.Broadcast(FName(*EquipmentIdStr), FName(*SensorKeyStr), static_cast<float>(Value), Timestamp);
}

void FWebSocketSensorSource::ScheduleReconnect()
{
	ClearReconnectTimer();

	// TODO: back off progressively (e.g. 3s/10s/30s) instead of a fixed 3s retry once
	// this needs to tolerate longer outages of the simulator / future OPC UA bridge.
	ReconnectTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([this](float /*DeltaTime*/) -> bool
		{
			ReconnectTickerHandle.Reset();
			Connect();
			return false; // one-shot
		}),
		ReconnectDelaySeconds);
}

void FWebSocketSensorSource::ClearReconnectTimer()
{
	if (ReconnectTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ReconnectTickerHandle);
		ReconnectTickerHandle.Reset();
	}
}
