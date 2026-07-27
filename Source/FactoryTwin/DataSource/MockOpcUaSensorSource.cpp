// Copyright Epic Games, Inc. All Rights Reserved.

#include "MockOpcUaSensorSource.h"

FMockOpcUaSensorSource::FMockOpcUaSensorSource(const FString& InEndpointUrl)
	: EndpointUrl(InEndpointUrl)
{
}

FMockOpcUaSensorSource::~FMockOpcUaSensorSource()
{
	Disconnect();
}

void FMockOpcUaSensorSource::Connect()
{
	if (bConnected)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MockOpcUaSensorSource] \"Connecting\" to %s (simulated -- no real OPC UA client wired up yet)"), *EndpointUrl);
	bConnected = true;

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateRaw(this, &FMockOpcUaSensorSource::Tick), 1.0f);
}

void FMockOpcUaSensorSource::Disconnect()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
	bConnected = false;
}

bool FMockOpcUaSensorSource::IsConnected() const
{
	return bConnected;
}

bool FMockOpcUaSensorSource::Tick(float /*DeltaTime*/)
{
	TemperatureValue = FMath::Clamp(TemperatureValue + FMath::RandRange(-2.0f, 2.0f), 40.0f, 110.0f);
	PressureValue = FMath::Clamp(PressureValue + FMath::RandRange(-1.0f, 1.0f), 5.0f, 60.0f);

	const FDateTime Now = FDateTime::UtcNow();
	SensorDataDelegate.Broadcast(TEXT("EQ-01"), TEXT("temperature"), TemperatureValue, Now);
	SensorDataDelegate.Broadcast(TEXT("EQ-01"), TEXT("pressure"), PressureValue, Now);

	return true; // keep ticking
}
