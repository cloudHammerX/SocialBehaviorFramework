// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class USBF_BehaviorMap;
class IDetailsView;
class SObjectPropertyEntryBox;
class SErrorText;

/**
 * Painted day/week timeline preview of a behavior map: 7 rows (Monday ..
 * Sunday) x 24 columns (hours). Colors: blue = Work, yellow = Leisure,
 * gray = Home. Weekend rows skip work when weekdays are enabled in the
 * project settings.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_TimelinePreview : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SSBF_TimelinePreview)
		: _BehaviorMap(nullptr)
		{}
		SLATE_ARGUMENT(const USBF_BehaviorMap*, BehaviorMap)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Swaps the displayed map and repaints. */
	void SetBehaviorMap(const USBF_BehaviorMap* InMap);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	const USBF_BehaviorMap* BehaviorMap = nullptr;

	static constexpr float CellWidth = 14.0f;
	static constexpr float RowHeight = 18.0f;
};

/**
 * "Behavior Map" tab of the unified editor window.
 *
 * Toolbar: map asset picker, New Map / Save / Revert / Validate buttons.
 * Body: the day/week timeline preview plus a full details view of the
 * selected USBF_BehaviorMap (locations, work hours, leisure overrides).
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_BehaviorMapTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSBF_BehaviorMapTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Reloads the selected map from disk (revert support). */
	void RefreshSelection();

	/** Validates the selected map; shows errors inline. @return true when valid. */
	bool ValidateSelection();

	/** Sets the edited map asset. */
	void SetMap(USBF_BehaviorMap* NewMap);

	/** @return the currently edited map. */
	USBF_BehaviorMap* GetMap() const { return CurrentMap; }

private:
	TSharedRef<SWidget> BuildToolbar();

	FString GetMapPath() const;
	void OnMapPicked(const FAssetData& AssetData);

	FReply OnNewMapClicked();
	FReply OnSaveClicked();
	FReply OnRevertClicked();
	FReply OnValidateClicked();

	USBF_BehaviorMap* CurrentMap = nullptr;
	FString CurrentMapPath;

	TSharedPtr<SObjectPropertyEntryBox> MapPicker;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SSBF_TimelinePreview> TimelinePreview;
	TSharedPtr<SErrorText> ValidationText;
};
