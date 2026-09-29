// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SBF_Types.h"
#include "Widgets/SBF_GraphNodeWidget.h"

class USBF_SocialGraph;
class USBF_SocialRelation;
class SObjectPropertyEntryBox;
class SErrorText;

/** Fired on the canvas' own context menu request (right click on empty space). */
DECLARE_DELEGATE_OneParam(FOnSBF_CanvasContextMenu, FVector2D /*ScreenPosition*/);

/**
 * Slate panel hosting the Social Graph nodes.
 *
 * Paints hierarchy + explicit relation edges (colored by resolved hostility:
 * green ally / red enemy / gray neutral), arranges the node widgets at their
 * stored positions and forwards context menu requests to the tab.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_GraphCanvas : public SPanel
{
public:
	SLATE_BEGIN_ARGS(SSBF_GraphCanvas)
		: _Graph(nullptr)
		{}
		SLATE_ARGUMENT(const USBF_SocialGraph*, Graph)
		SLATE_EVENT(FOnSBF_NodeContextMenu, OnNodeContextMenu)
		SLATE_EVENT(FOnSBF_CanvasContextMenu, OnCanvasContextMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Rebuilds the node widgets from the given graph (keeps stored positions). */
	void RebuildGraph(const USBF_SocialGraph* InGraph);

	/** Updates the stored position of a node (drag callback). */
	void SetNodePosition(FGuid NodeGuid, FVector2D Position);

	/** @return the stored position of a node (nullptr when unknown). */
	const FVector2D* GetNodePosition(FGuid NodeGuid) const;

	/** Removes all node widgets. */
	void ClearCanvas();

	/** Distributes all nodes on a circle layout. */
	void ArrangeCircleLayout();

	// ~SPanel
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual FChildren* GetChildren() override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot(TSharedRef<SWidget> InWidget)
			: TSlotBase<FSlot>(InWidget)
		{
		}
		FGuid NodeGuid;
	};

	FSlot& AddSlot(TSharedRef<SWidget> InWidget, FGuid NodeGuid);
	void HandleNodeMoved(FGuid NodeGuid, FVector2D NewPosition);

	TPanelChildren<FSlot> Children;
	TMap<FGuid, FVector2D> NodePositions;
	const USBF_SocialGraph* Graph = nullptr;

	FOnSBF_NodeContextMenu OnNodeContextMenu;
	FOnSBF_CanvasContextMenu OnCanvasContextMenu;
};

/**
 * "Social Graph" tab of the unified editor window.
 *
 * Toolbar: graph asset picker, New Graph / New Relation / Save / Revert /
 * Validate / Arrange buttons. Canvas: node widgets with context menus for
 * renaming, relation assignment, parenting, edge creation, actor binding and
 * deletion.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_SocialGraphTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSBF_SocialGraphTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Reloads the selected graph from disk (revert support). */
	void RefreshSelection();

	/** Validates the selected graph; shows errors inline. @return true when valid. */
	bool ValidateSelection();

	/** Sets the edited graph asset. */
	void SetGraph(USBF_SocialGraph* NewGraph);

	/** @return the currently edited graph. */
	USBF_SocialGraph* GetGraph() const { return CurrentGraph; }

	/** @return a hostility-driven color (used by the canvas edge painter). */
	static FLinearColor GetHostilityColor(ESBF_Hostility Hostility);

private:
	TSharedRef<SWidget> BuildToolbar();

	FString GetGraphPath() const;
	void OnGraphPicked(const FAssetData& AssetData);

	FReply OnNewGraphClicked();
	FReply OnNewRelationClicked();
	FReply OnSaveClicked();
	FReply OnRevertClicked();
	FReply OnValidateClicked();
	FReply OnArrangeClicked();

	void RebuildCanvas();

	void HandleNodeContextMenu(FGuid NodeGuid, FVector2D ScreenPosition);
	void HandleCanvasContextMenu(FVector2D ScreenPosition);
	TSharedRef<SWidget> BuildNodeMenu(FGuid NodeGuid);
	TSharedRef<SWidget> BuildCanvasMenu();

	void AddNode();
	void DeleteNode(FGuid NodeGuid);
	void AssignRelationToNode(FGuid NodeGuid, USBF_SocialRelation* Relation);
	void SetNodeParent(FGuid NodeGuid, FGuid NewParentGuid);
	void CreateEdge(FGuid NodeA, FGuid NodeB);
	void BindNodeToSelection(FGuid NodeGuid);
	void RenameNode(FGuid NodeGuid);
	bool PromptForText(const FText& Title, const FText& InitialValue, FText& OutValue) const;

	USBF_SocialGraph* CurrentGraph = nullptr;
	FString CurrentGraphPath;

	TSharedPtr<SSBF_GraphCanvas> Canvas;
	TSharedPtr<SObjectPropertyEntryBox> GraphPicker;
	TSharedPtr<SErrorText> ValidationText;
};
