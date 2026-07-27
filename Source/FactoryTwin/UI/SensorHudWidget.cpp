// Copyright Epic Games, Inc. All Rights Reserved.

#include "SensorHudWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SensorSubsystem.h"

void USensorHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildLayout();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USensorSubsystem* SensorSubsystem = GameInstance->GetSubsystem<USensorSubsystem>())
		{
			SensorSubsystem->OnSensorDataReceived().AddUObject(this, &USensorHudWidget::HandleSensorData);
		}
	}

	RefreshDisplay();
}

void USensorHudWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USensorSubsystem* SensorSubsystem = GameInstance->GetSubsystem<USensorSubsystem>())
		{
			SensorSubsystem->OnSensorDataReceived().RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void USensorHudWidget::BuildLayout()
{
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UVerticalBox* ReadoutPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReadoutPanel"));
	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(ReadoutPanel))
	{
		PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		PanelSlot->SetPosition(FVector2D(40.0f, 40.0f));
		PanelSlot->SetAutoSize(true);
	}

	auto MakeLine = [this, ReadoutPanel](const FString& InitialText) -> UTextBlock*
	{
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		TextBlock->SetText(FText::FromString(InitialText));

		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = 20;
		TextBlock->SetFont(FontInfo);
		TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));

		if (UVerticalBoxSlot* LineSlot = ReadoutPanel->AddChildToVerticalBox(TextBlock))
		{
			LineSlot->SetPadding(FMargin(0.0f, 2.0f));
		}
		return TextBlock;
	};

	TitleText = MakeLine(EquipmentId.ToString());
	TemperatureText = MakeLine(TEXT("Temp: --"));
	PressureText = MakeLine(TEXT("Press: --"));
}

void USensorHudWidget::HandleSensorData(FName InEquipmentId, FName SensorKey, float Value, FDateTime Timestamp)
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

	RefreshDisplay();
}

void USensorHudWidget::RefreshDisplay()
{
	TitleText->SetText(FText::FromString(EquipmentId.ToString()));

	FLinearColor StatusColor = FLinearColor::Gray;
	FString TemperatureString = TEXT("--");
	if (LatestTemperature.IsSet())
	{
		const float Temperature = LatestTemperature.GetValue();
		TemperatureString = FString::Printf(TEXT("%.1f"), Temperature);

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
	TemperatureText->SetText(FText::FromString(FString::Printf(TEXT("Temp: %s"), *TemperatureString)));
	TemperatureText->SetColorAndOpacity(FSlateColor(StatusColor));

	const FString PressureString = LatestPressure.IsSet()
		? FString::Printf(TEXT("%.1f"), LatestPressure.GetValue())
		: TEXT("--");
	PressureText->SetText(FText::FromString(FString::Printf(TEXT("Press: %s"), *PressureString)));
}
