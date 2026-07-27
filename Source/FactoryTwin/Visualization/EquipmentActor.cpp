// Copyright Epic Games, Inc. All Rights Reserved.

#include "EquipmentActor.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SensorSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AEquipmentActor::AEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComponent"));
	SetRootComponent(BodyMeshComponent);
	BodyMeshComponent->SetMobility(EComponentMobility::Movable);
	BodyMeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 2.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		BodyMeshComponent->SetStaticMesh(CubeMeshAsset.Object);
	}

	StatusLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLightComponent"));
	StatusLightComponent->SetupAttachment(BodyMeshComponent);
	StatusLightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	StatusLightComponent->SetIntensity(5000.0f);
	StatusLightComponent->SetLightColor(FLinearColor::Gray);
	StatusLightComponent->SetAttenuationRadius(400.0f);

	ReadoutTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ReadoutTextComponent"));
	ReadoutTextComponent->SetupAttachment(BodyMeshComponent);
	ReadoutTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 220.0f));
	ReadoutTextComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	ReadoutTextComponent->SetHorizontalAlignment(EHTA_Center);
	ReadoutTextComponent->SetWorldSize(32.0f);
	ReadoutTextComponent->SetText(FText::FromString(TEXT("--")));
}

void AEquipmentActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USensorSubsystem* SensorSubsystem = GameInstance->GetSubsystem<USensorSubsystem>())
		{
			SensorSubsystem->OnSensorDataReceived().AddUObject(this, &AEquipmentActor::HandleSensorData);
		}
	}

	RefreshVisuals();
}

void AEquipmentActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USensorSubsystem* SensorSubsystem = GameInstance->GetSubsystem<USensorSubsystem>())
		{
			SensorSubsystem->OnSensorDataReceived().RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AEquipmentActor::HandleSensorData(FName InEquipmentId, FName SensorKey, float Value, FDateTime Timestamp)
{
	if (InEquipmentId != EquipmentId)
	{
		return;
	}

	if (SensorKey == TEXT("temperature"))
	{
		LatestTemperature = Value;
	}
	else if (SensorKey == TEXT("pressure"))
	{
		LatestPressure = Value;
	}
	else
	{
		return;
	}

	RefreshVisuals();
}

void AEquipmentActor::RefreshVisuals()
{
	FLinearColor StatusColor = FLinearColor::Gray;
	if (LatestTemperature.IsSet())
	{
		const float Temperature = LatestTemperature.GetValue();
		if (Temperature >= CriticalTemperature)
		{
			StatusColor = FLinearColor::Red;
		}
		else if (Temperature >= WarningTemperature)
		{
			StatusColor = FLinearColor(1.0f, 0.65f, 0.0f); // amber
		}
		else
		{
			StatusColor = FLinearColor::Green;
		}
	}
	StatusLightComponent->SetLightColor(StatusColor);

	const FString TemperatureText = LatestTemperature.IsSet()
		? FString::Printf(TEXT("%.1f"), LatestTemperature.GetValue())
		: TEXT("--");
	const FString PressureText = LatestPressure.IsSet()
		? FString::Printf(TEXT("%.1f"), LatestPressure.GetValue())
		: TEXT("--");

	ReadoutTextComponent->SetText(FText::FromString(FString::Printf(
		TEXT("%s\nTemp: %s\nPress: %s"), *EquipmentId.ToString(), *TemperatureText, *PressureText)));
}
