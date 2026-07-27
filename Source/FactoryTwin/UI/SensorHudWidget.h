// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "SensorHudWidget.generated.h"

class UTextBlock;

/**
 * Minimal on-screen HUD showing live sensor readings for one piece of equipment.
 * Entire widget tree is built in C++ (NativeConstruct) rather than a Widget
 * Blueprint asset, so the whole slice stays reproducible from source alone —
 * no manual UMG designer work needed.
 */
UCLASS()
class FACTORYTWIN_API USensorHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FactoryTwin")
	FName EquipmentId = TEXT("EQ-01");

	// Vertical pixel offset so multiple equipment HUDs can be stacked without overlapping.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FactoryTwin")
	float ScreenOffsetY = 0.0f;

protected:
	// Builds the widget tree here rather than in NativeConstruct(): RebuildWidget() runs
	// BEFORE NativeConstruct(), and UUserWidget::RebuildWidget() snapshots WidgetTree->RootWidget
	// into the actual SWidget at that point -- if it's still null (as it would be if we built
	// the tree in NativeConstruct instead), you silently get an empty SSpacer with no visible content.
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BuildLayout();
	void HandleSensorData(FName InEquipmentId, FName SensorKey, float Value, FDateTime Timestamp);
	void RefreshDisplay();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TemperatureText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PressureText;

	UPROPERTY(Transient)
	TWeakObjectPtr<class USensorSubsystem> CachedSensorSubsystem;

	TOptional<float> LatestTemperature;
	TOptional<float> LatestPressure;
};
