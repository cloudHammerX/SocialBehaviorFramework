// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "GameplayTagContainer.h"

/** Fired when a graph node was dragged to a new canvas position. */
DECLARE_DELEGATE_TwoParams(FOnSBF_NodePositionChanged, FGuid /*NodeGuid*/, FVector2D /*NewPosition*/);

/** Fired on the node's context menu request (right click). */
DECLARE_DELEGATE_TwoParams(FOnSBF_NodeContextMenu, FGuid /*NodeGuid*/, FVector2D /*ScreenPosition*/);

/**
 * Slate widget representing a single Social Graph node on the editor canvas.
 *
 * Draggable (left mouse), shows the node display name and its relation tag
 * with a color-coded border; forwards the context menu request to the tab.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_GraphNodeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSBF_GraphNodeWidget)
		: _NodeGuid()
		, _DisplayName(NAME_None)
		, _NodeColor(FLinearColor(0.35f, 0.35f, 0.4f))
		, _InitialOffset(FVector2D::ZeroVector)
		{}
		SLATE_ARGUMENT(FGuid, NodeGuid)
		SLATE_ARGUMENT(FName, DisplayName)
		SLATE_ARGUMENT(FGameplayTag, RelationTag)
		SLATE_ARGUMENT(FLinearColor, NodeColor)
		SLATE_ARGUMENT(FVector2D, InitialOffset)
		SLATE_EVENT(FOnSBF_NodePositionChanged, OnPositionChanged)
		SLATE_EVENT(FOnSBF_NodeContextMenu, OnContextMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	FGuid NodeGuid;
	FVector2D CurrentOffset = FVector2D::ZeroVector;
	FVector2D DragStartScreenPos = FVector2D::ZeroVector;
	FVector2D DragStartOffset = FVector2D::ZeroVector;
	bool bDragging = false;

	FOnSBF_NodePositionChanged OnPositionChanged;
	FOnSBF_NodeContextMenu OnContextMenu;
};
