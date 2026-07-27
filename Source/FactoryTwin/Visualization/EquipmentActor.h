// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/Optional.h"
#include "EquipmentActor.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Minimal 3D stand-in for a piece of factory equipment. Subscribes to
 * USensorSubsystem::OnSensorDataReceived and reacts to readings for its own
 * EquipmentId: a status light colored green/amber/red by temperature, and an
 * in-world text readout of the latest values. No custom materials or level
 * placement required — everything is code-only so it works the moment PIE starts.
 */
UCLASS()
class FACTORYTWIN_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

public:
	AEquipmentActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FactoryTwin")
	FName EquipmentId = TEXT("EQ-01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FactoryTwin")
	float WarningTemperature = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FactoryTwin")
	float CriticalTemperature = 100.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleSensorData(FName InEquipmentId, FName SensorKey, float Value, FDateTime Timestamp);
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, Category = "FactoryTwin")
	TObjectPtr<UStaticMeshComponent> BodyMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "FactoryTwin")
	TObjectPtr<UPointLightComponent> StatusLightComponent;

	// Title + Temp: colored by StatusColor (temperature drives overall equipment status).
	UPROPERTY(VisibleAnywhere, Category = "FactoryTwin")
	TObjectPtr<UTextRenderComponent> StatusTextComponent;

	// Press: always neutral white — pressure is informational only, doesn't affect status color.
	UPROPERTY(VisibleAnywhere, Category = "FactoryTwin")
	TObjectPtr<UTextRenderComponent> PressureTextComponent;

	TOptional<float> LatestTemperature;
	TOptional<float> LatestPressure;
};
