// Copyright Social Behavior Framework. All Rights Reserved.

#include "Widgets/SBF_GraphNodeWidget.h"

#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

void SSBF_GraphNodeWidget::Construct(const FArguments& InArgs)
{
	NodeGuid = InArgs._NodeGuid;
	CurrentOffset = InArgs._InitialOffset;
	OnPositionChanged = InArgs._OnPositionChanged;
	OnContextMenu = InArgs._OnContextMenu;

	const FName DisplayName = InArgs._DisplayName;
	const FGameplayTag RelationTag = InArgs._RelationTag;
	const FLinearColor NodeColor = InArgs._NodeColor;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(NodeColor)
		.Padding(FMargin(6.0f, 4.0f))
		.ToolTipText(FText::FromString(RelationTag.IsValid() ? RelationTag.ToString() : TEXT("No relation")))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(DisplayName))
				.Font(FAppStyle::GetFontStyle("NormalFontBold"))
				.ColorAndOpacity(FLinearColor::White)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(RelationTag.IsValid() ? RelationTag.GetTagName().ToString() : TEXT("-")))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f))
			]
		]
	];
}

FReply SSBF_GraphNodeWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OnContextMenu.ExecuteIfBound(NodeGuid, MouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bDragging = true;
		DragStartScreenPos = MouseEvent.GetScreenSpacePosition();
		DragStartOffset = CurrentOffset;
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	return FReply::Unhandled();
}

FReply SSBF_GraphNodeWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bDragging)
	{
		const FVector2D Delta = MouseEvent.GetScreenSpacePosition() - DragStartScreenPos;
		CurrentOffset = DragStartOffset + Delta;
		OnPositionChanged.ExecuteIfBound(NodeGuid, CurrentOffset);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SSBF_GraphNodeWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}
