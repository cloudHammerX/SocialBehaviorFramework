// Copyright Social Behavior Framework. All Rights Reserved.

#include "Widgets/SBF_BehaviorMapTab.h"

#include "Framework/Application/SlateApplication.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SObjectPropertyEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/SErrorText.h"
#include "Widgets/Text/STextBlock.h"
#include "SBF_AssetFactories.h"
#include "SBF_BehaviorMap.h"
#include "SBF_DeveloperSettings.h"
#include "SBF_EditorWindow.h"
#include "SBF_Log.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "SSBF_BehaviorMapTab"

// =============================================================================
// SSBF_TimelinePreview
// =============================================================================

void SSBF_TimelinePreview::Construct(const FArguments& InArgs)
{
	BehaviorMap = InArgs._BehaviorMap;
}

void SSBF_TimelinePreview::SetBehaviorMap(const USBF_BehaviorMap* InMap)
{
	BehaviorMap = InMap;
	Invalidate(EInvalidateWidget::Paint);
}

int32 SSBF_TimelinePreview::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	static const FName WeekdayNames[7] =
	{
		FName(TEXT("Monday")), FName(TEXT("Tuesday")), FName(TEXT("Wednesday")),
		FName(TEXT("Thursday")), FName(TEXT("Friday")), FName(TEXT("Saturday")), FName(TEXT("Sunday"))
	};

	const FSlateBrush* WhiteBrush = FAppStyle::GetBrush("WhiteBrush");
	const USBF_DeveloperSettings& Settings = USBF_DeveloperSettings::Get();

	// Background.
	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(),
		WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.06f, 0.06f, 0.08f));

	if (!BehaviorMap)
	{
		return LayerId + 1;
	}

	for (int32 Day = 0; Day < 7; ++Day)
	{
		const bool bWeekend = Settings.bEnableWeekdays && Settings.WeekendDays.Contains(WeekdayNames[Day]);

		for (int32 Hour = 0; Hour < 24; ++Hour)
		{
			FSBF_GameTime Time;
			Time.Hour = Hour;
			Time.Minute = 0;
			Time.Day = Day + 1;
			Time.Weekday = WeekdayNames[Day];
			Time.bIsWeekend = bWeekend;

			ESBF_ActivityType Activity = ESBF_ActivityType::None;
			FVector Location;
			BehaviorMap->ResolveCurrentActivity(Time, Activity, Location);

			FLinearColor CellColor = FLinearColor(0.16f, 0.16f, 0.18f);
			switch (Activity)
			{
			case ESBF_ActivityType::Work:
				CellColor = FLinearColor(0.10f, 0.35f, 0.85f);
				break;
			case ESBF_ActivityType::Leisure:
				CellColor = FLinearColor(0.85f, 0.70f, 0.10f);
				break;
			case ESBF_ActivityType::Home:
				CellColor = FLinearColor(0.40f, 0.40f, 0.42f);
				break;
			default:
				break;
			}

			const FVector2D Origin(4.0f + Hour * CellWidth, 4.0f + Day * RowHeight);
			FSlateDrawElement::MakeBox(
				OutDrawElements, LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2D(CellWidth - 1.0f, RowHeight - 1.0f), FSlateLayoutTransform(Origin)),
				WhiteBrush, ESlateDrawEffect::None, CellColor);
		}
	}

	return LayerId + 1;
}

FVector2D SSBF_TimelinePreview::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(24.0f * CellWidth + 8.0f, 7.0f * RowHeight + 8.0f);
}

// =============================================================================
// SSBF_BehaviorMapTab
// =============================================================================

void SSBF_BehaviorMapTab::Construct(const FArguments& InArgs)
{
	const USBF_BehaviorMap* DefaultMap = USBF_DeveloperSettings::Get().DefaultBehaviorMap.LoadSynchronous();
	CurrentMap = const_cast<USBF_BehaviorMap*>(DefaultMap);
	if (CurrentMap)
	{
		CurrentMapPath = CurrentMap->GetPathName();
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bUpdatesFromSelection = false;
	DetailsArgs.bLockable = false;
	DetailsArgs.bAllowSearch = true;
	DetailsArgs.bShowOptions = true;
	DetailsView = PropertyModule.CreateDetailView(DetailsArgs);
	if (CurrentMap)
	{
		DetailsView->SetObject(CurrentMap);
	}

	SAssignNew(ValidationText, SErrorText);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			BuildToolbar()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 0.0f)
		[
			ValidationText.ToSharedRef()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SBox)
			.HeightOverride(150.0f)
			[
				SAssignNew(TimelinePreview, SSBF_TimelinePreview)
				.BehaviorMap(CurrentMap)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(4.0f)
		[
			DetailsView.ToSharedRef()
		]
	];
}

TSharedRef<SWidget> SSBF_BehaviorMapTab::BuildToolbar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MapLabel", "Map:"))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SAssignNew(MapPicker, SObjectPropertyEntryBox)
			.AllowedClass(USBF_BehaviorMap::StaticClass())
			.AllowClear(true)
			.ObjectPath_Lambda([this]() { return GetMapPath(); })
			.OnObjectChanged(this, &SSBF_BehaviorMapTab::OnMapPicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("NewMap", "New Map"))
			.OnClicked(this, &SSBF_BehaviorMapTab::OnNewMapClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Save", "Save"))
			.OnClicked(this, &SSBF_BehaviorMapTab::OnSaveClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Revert", "Revert"))
			.OnClicked(this, &SSBF_BehaviorMapTab::OnRevertClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Validate", "Validate"))
			.OnClicked(this, &SSBF_BehaviorMapTab::OnValidateClicked)
		];
}

FString SSBF_BehaviorMapTab::GetMapPath() const
{
	return CurrentMapPath;
}

void SSBF_BehaviorMapTab::OnMapPicked(const FAssetData& AssetData)
{
	UObject* Asset = AssetData.GetAsset();
	if (!Asset)
	{
		SetMap(nullptr);
		return;
	}

	USBF_BehaviorMap* Map = Cast<USBF_BehaviorMap>(Asset);
	if (!Map)
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: selected asset is not a USBF_BehaviorMap."));
		return;
	}
	SetMap(Map);
}

void SSBF_BehaviorMapTab::SetMap(USBF_BehaviorMap* NewMap)
{
	CurrentMap = NewMap;
	CurrentMapPath = NewMap ? NewMap->GetPathName() : FString();

	if (DetailsView)
	{
		DetailsView->SetObject(NewMap);
	}
	if (TimelinePreview)
	{
		TimelinePreview->SetBehaviorMap(NewMap);
	}
}

void SSBF_BehaviorMapTab::RefreshSelection()
{
	if (CurrentMapPath.IsEmpty())
	{
		return;
	}
	SetMap(LoadObject<USBF_BehaviorMap>(nullptr, *CurrentMapPath));
}

bool SSBF_BehaviorMapTab::ValidateSelection()
{
	if (!CurrentMap)
	{
		ValidationText->SetError(LOCTEXT("NoMapSelected", "No Behavior Map selected."));
		return false;
	}

	TArray<FText> Errors;
	const bool bValid = CurrentMap->Validate(Errors);
	if (bValid)
	{
		ValidationText->SetError(FText::GetEmpty());
		return true;
	}

	FString Joined;
	for (const FText& Error : Errors)
	{
		Joined += Error.ToString();
		Joined += TEXT("\n");
		UE_LOG(LogSBF, Warning, TEXT("SBF validation: %s"), *Error.ToString());
	}
	ValidationText->SetError(FText::FromString(Joined));
	return false;
}

FReply SSBF_BehaviorMapTab::OnNewMapClicked()
{
	USBF_BehaviorMap* NewMap = SSBF_EditorWindow::CreateAsset<USBF_BehaviorMap>(
		TEXT("SBF_BehaviorMap"), TEXT("/Game/SBF"), USBF_BehaviorMapFactory::StaticClass());
	if (NewMap)
	{
		SetMap(NewMap);
	}
	else
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: failed to create a new Behavior Map."));
	}
	return FReply::Handled();
}

FReply SSBF_BehaviorMapTab::OnSaveClicked()
{
	SSBF_EditorWindow::SaveAsset(CurrentMap);
	return FReply::Handled();
}

FReply SSBF_BehaviorMapTab::OnRevertClicked()
{
	if (CurrentMap)
	{
		SetMap(Cast<USBF_BehaviorMap>(SSBF_EditorWindow::RevertAsset(CurrentMap)));
	}
	return FReply::Handled();
}

FReply SSBF_BehaviorMapTab::OnValidateClicked()
{
	ValidateSelection();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
