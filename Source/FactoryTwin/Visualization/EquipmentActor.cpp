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

	StatusTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusTextComponent"));
	StatusTextComponent->SetupAttachment(BodyMeshComponent);
	StatusTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 240.0f));
	StatusTextComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	StatusTextComponent->SetHorizontalAlignment(EHTA_Center);
	StatusTextComponent->SetWorldSize(32.0f);
	StatusTextComponent->SetText(FText::FromString(TEXT("--")));

	// Separate component so pressure's text never inherits the temperature-driven status color.
	PressureTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PressureTextComponent"));
	PressureTextComponent->SetupAttachment(BodyMeshComponent);
	PressureTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 190.0f));
	PressureTextComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	PressureTextComponent->SetHorizontalAlignment(EHTA_Center);
	PressureTextComponent->SetWorldSize(32.0f);
	PressureTextComponent->SetTextRenderColor(FColor::White);
	PressureTextComponent->SetText(FText::FromString(TEXT("Press: --")));
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
	StatusTextComponent->SetTextRenderColor(StatusColor.ToFColor(true));

	const FString TemperatureText = LatestTemperature.IsSet()
		? FString::Printf(TEXT("%.1f"), LatestTemperature.GetValue())
		: TEXT("--");
	StatusTextComponent->SetText(FText::FromString(FString::Printf(
		TEXT("%s\nTemp: %s"), *EquipmentId.ToString(), *TemperatureText)));

	// Pressure stays neutral white regardless of status — it's informational only.
	const FString PressureText = LatestPressure.IsSet()
		? FString::Printf(TEXT("%.1f"), LatestPressure.GetValue())
		: TEXT("--");
	PressureTextComponent->SetText(FText::FromString(FString::Printf(TEXT("Press: %s"), *PressureText)));
}
