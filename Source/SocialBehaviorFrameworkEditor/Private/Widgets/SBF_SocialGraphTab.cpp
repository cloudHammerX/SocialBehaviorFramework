// Copyright Social Behavior Framework. All Rights Reserved.

#include "Widgets/SBF_SocialGraphTab.h"

#include "AssetToolsModule.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "InputCoreTypes.h"
#include "Math/UnrealMathUtility.h"
#include "ObjectTools.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "Selection.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SObjectPropertyEntryBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/SErrorText.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBF_GraphNodeWidget.h"
#include "SBF_AssetFactories.h"
#include "SBF_DeveloperSettings.h"
#include "SBF_EditorWindow.h"
#include "SBF_Log.h"
#include "SBF_SocialGraph.h"
#include "SBF_SocialRelation.h"

#define LOCTEXT_NAMESPACE "SSBF_SocialGraphTab"

// =============================================================================
// SSBF_GraphCanvas
// =============================================================================

void SSBF_GraphCanvas::Construct(const FArguments& InArgs)
{
	Graph = InArgs._Graph;
	OnNodeContextMenu = InArgs._OnNodeContextMenu;
	OnCanvasContextMenu = InArgs._OnCanvasContextMenu;
}

void SSBF_GraphCanvas::RebuildGraph(const USBF_SocialGraph* InGraph)
{
	Graph = InGraph;
	Children.Empty();

	if (!Graph)
	{
		return;
	}

	const int32 NodeCount = FMath::Max(1, Graph->Nodes.Num());
	for (int32 Index = 0; Index < Graph->Nodes.Num(); ++Index)
	{
		const FSBF_SocialNode& Node = Graph->Nodes[Index];

		if (!NodePositions.Contains(Node.NodeGuid))
		{
			const float Angle = static_cast<float>(Index) * 2.0f * PI / static_cast<float>(NodeCount);
			NodePositions.Add(Node.NodeGuid, FVector2D(420.0f + FMath::Cos(Angle) * 260.0f, 320.0f + FMath::Sin(Angle) * 220.0f));
		}

		TSharedRef<SSBF_GraphNodeWidget> NodeWidget = SNew(SSBF_GraphNodeWidget)
			.NodeGuid(Node.NodeGuid)
			.DisplayName(Node.DisplayName)
			.RelationTag(Node.RelationTag)
			.NodeColor(FLinearColor(0.30f, 0.32f, 0.38f))
			.InitialOffset(NodePositions[Node.NodeGuid])
			.OnPositionChanged(this, &SSBF_GraphCanvas::HandleNodeMoved)
			.OnContextMenu(OnNodeContextMenu);

		AddSlot(NodeWidget, Node.NodeGuid);
	}

	Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void SSBF_GraphCanvas::SetNodePosition(FGuid NodeGuid, FVector2D Position)
{
	NodePositions.Add(NodeGuid, Position);
	Invalidate(EInvalidateWidget::Paint);
}

const FVector2D* SSBF_GraphCanvas::GetNodePosition(FGuid NodeGuid) const
{
	return NodePositions.Find(NodeGuid);
}

void SSBF_GraphCanvas::ClearCanvas()
{
	Children.Empty();
	NodePositions.Empty();
	Graph = nullptr;
	Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void SSBF_GraphCanvas::ArrangeCircleLayout()
{
	if (!Graph)
	{
		return;
	}

	const int32 NodeCount = FMath::Max(1, Graph->Nodes.Num());
	for (int32 Index = 0; Index < Graph->Nodes.Num(); ++Index)
	{
		const float Angle = static_cast<float>(Index) * 2.0f * PI / static_cast<float>(NodeCount);
		NodePositions.Add(Graph->Nodes[Index].NodeGuid, FVector2D(420.0f + FMath::Cos(Angle) * 260.0f, 320.0f + FMath::Sin(Angle) * 220.0f));
	}

	Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

SSBF_GraphCanvas::FSlot& SSBF_GraphCanvas::AddSlot(TSharedRef<SWidget> InWidget, FGuid NodeGuid)
{
	FSlot* NewSlot = new FSlot(InWidget);
	NewSlot->NodeGuid = NodeGuid;
	Children.Add(NewSlot);
	return *NewSlot;
}

void SSBF_GraphCanvas::HandleNodeMoved(FGuid NodeGuid, FVector2D NewPosition)
{
	NodePositions.Add(NodeGuid, NewPosition);
	Invalidate(EInvalidateWidget::Paint);
}

void SSBF_GraphCanvas::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
	{
		const FSlot& CurChild = Children[ChildIndex];
		const FVector2D* StoredPos = NodePositions.Find(CurChild.NodeGuid);
		const FVector2D Offset = StoredPos ? *StoredPos : FVector2D(40.0f, 40.0f);

		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(CurChild.GetWidget(), Offset));
	}
}

int32 SSBF_GraphCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (Graph)
	{
		// Hierarchy edges (node -> parent), colored by resolved hostility.
		for (const FSBF_SocialNode& Node : Graph->Nodes)
		{
			if (!Node.ParentGuid.IsValid())
			{
				continue;
			}

			FSBF_SocialNode Parent;
			if (!Graph->FindNode(Node.ParentGuid, Parent))
			{
				continue;
			}

			const FVector2D* From = NodePositions.Find(Node.NodeGuid);
			const FVector2D* To = NodePositions.Find(Parent.NodeGuid);
			if (!From || !To)
			{
				continue;
			}

			const ESBF_Hostility Hostility = Graph->ResolveHostility(Node.NodeGuid, Parent.NodeGuid);
			const FLinearColor Color = SSBF_SocialGraphTab::GetHostilityColor(Hostility);

			const TArray<FVector2D> Points = { *From + FVector2D(60.0f, 20.0f), *To + FVector2D(60.0f, 20.0f) };
			FSlateDrawElement::MakeLines(
				OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points,
				ESlateDrawEffect::None, Color, /*bAntialias=*/true, 2.0f);
		}

		// Explicit edges (thicker).
		for (const TPair<FGuid, FSBF_SocialEdge>& Pair : Graph->Edges)
		{
			const FSBF_SocialEdge& Edge = Pair.Value;
			FSBF_SocialNode NodeA, NodeB;
			if (!Graph->FindNode(Edge.NodeA, NodeA) || !Graph->FindNode(Edge.NodeB, NodeB))
			{
				continue;
			}

			const FVector2D* From = NodePositions.Find(Edge.NodeA);
			const FVector2D* To = NodePositions.Find(Edge.NodeB);
			if (!From || !To)
			{
				continue;
			}

			const ESBF_Hostility Hostility = Graph->ResolveHostility(Edge.NodeA, Edge.NodeB);
			const FLinearColor Color = SSBF_SocialGraphTab::GetHostilityColor(Hostility);

			const TArray<FVector2D> Points = { *From + FVector2D(60.0f, 20.0f), *To + FVector2D(60.0f, 20.0f) };
			FSlateDrawElement::MakeLines(
				OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points,
				ESlateDrawEffect::None, Color, /*bAntialias=*/true, 4.0f);
		}
	}

	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	OnArrangeChildren(AllottedGeometry, ArrangedChildren);
	return PaintArrangedChildren(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId + 1, InWidgetStyle, bParentEnabled, ArrangedChildren);
}

FVector2D SSBF_GraphCanvas::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1600.0f, 1000.0f);
}

FChildren* SSBF_GraphCanvas::GetChildren()
{
	return &Children;
}

FReply SSBF_GraphCanvas::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OnCanvasContextMenu.ExecuteIfBound(MouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

// =============================================================================
// SSBF_SocialGraphTab
// =============================================================================

void SSBF_SocialGraphTab::Construct(const FArguments& InArgs)
{
	// Preselect the project default graph when available.
	const USBF_SocialGraph* DefaultGraph = USBF_DeveloperSettings::Get().DefaultSocialGraph.LoadSynchronous();

	CurrentGraph = const_cast<USBF_SocialGraph*>(DefaultGraph);
	if (CurrentGraph)
	{
		CurrentGraphPath = CurrentGraph->GetPathName();
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
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Horizontal)
			+ SScrollBox::Slot()
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				+ SScrollBox::Slot()
				[
					SAssignNew(Canvas, SSBF_GraphCanvas)
					.Graph(CurrentGraph)
					.OnNodeContextMenu(this, &SSBF_SocialGraphTab::HandleNodeContextMenu)
					.OnCanvasContextMenu(this, &SSBF_SocialGraphTab::HandleCanvasContextMenu)
				]
			]
		]
	];

	RebuildCanvas();
}

TSharedRef<SWidget> SSBF_SocialGraphTab::BuildToolbar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GraphLabel", "Graph:"))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SAssignNew(GraphPicker, SObjectPropertyEntryBox)
			.AllowedClass(USBF_SocialGraph::StaticClass())
			.AllowClear(true)
			.ObjectPath_Lambda([this]() { return GetGraphPath(); })
			.OnObjectChanged(this, &SSBF_SocialGraphTab::OnGraphPicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("NewGraph", "New Graph"))
			.ToolTipText(LOCTEXT("NewGraphTooltip", "Creates a new Social Graph data asset."))
			.OnClicked(this, &SSBF_SocialGraphTab::OnNewGraphClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("NewRelation", "New Relation"))
			.ToolTipText(LOCTEXT("NewRelationTooltip", "Creates a new relation asset and adds it to the current graph."))
			.OnClicked(this, &SSBF_SocialGraphTab::OnNewRelationClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Save", "Save"))
			.OnClicked(this, &SSBF_SocialGraphTab::OnSaveClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Revert", "Revert"))
			.OnClicked(this, &SSBF_SocialGraphTab::OnRevertClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Validate", "Validate"))
			.OnClicked(this, &SSBF_SocialGraphTab::OnValidateClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Arrange", "Arrange"))
			.ToolTipText(LOCTEXT("ArrangeTooltip", "Rearranges all nodes on a circle layout."))
			.OnClicked(this, &SSBF_SocialGraphTab::OnArrangeClicked)
		];
}

FString SSBF_SocialGraphTab::GetGraphPath() const
{
	return CurrentGraphPath;
}

void SSBF_SocialGraphTab::OnGraphPicked(const FAssetData& AssetData)
{
	UObject* Asset = AssetData.GetAsset();
	if (!Asset)
	{
		SetGraph(nullptr);
		return;
	}

	USBF_SocialGraph* Graph = Cast<USBF_SocialGraph>(Asset);
	if (!Graph)
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: selected asset is not a USBF_SocialGraph."));
		return;
	}
	SetGraph(Graph);
}

FReply SSBF_SocialGraphTab::OnNewGraphClicked()
{
	USBF_SocialGraph* NewGraph = SSBF_EditorWindow::CreateAsset<USBF_SocialGraph>(
		TEXT("SBF_Graph"), TEXT("/Game/SBF"), USBF_SocialGraphFactory::StaticClass());
	if (NewGraph)
	{
		SetGraph(NewGraph);
	}
	else
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: failed to create a new Social Graph (is /Game/SBF writable?)."));
	}
	return FReply::Handled();
}

FReply SSBF_SocialGraphTab::OnNewRelationClicked()
{
	if (!CurrentGraph)
	{
		ValidationText->SetError(LOCTEXT("NoGraphForRelation", "Select or create a Social Graph first."));
		return FReply::Handled();
	}

	USBF_SocialRelation* NewRelation = SSBF_EditorWindow::CreateAsset<USBF_SocialRelation>(
		FString::Printf(TEXT("SBF_Relation_%d"), CurrentGraph->RelationAssets.Num() + 1),
		TEXT("/Game/SBF"), USBF_SocialRelationFactory::StaticClass());
	if (NewRelation)
	{
		CurrentGraph->Modify();
		CurrentGraph->RelationAssets.Add(NewRelation);
		CurrentGraph->MarkPackageDirty();
	}
	else
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: failed to create a new relation asset."));
	}
	return FReply::Handled();
}

FReply SSBF_SocialGraphTab::OnSaveClicked()
{
	SSBF_EditorWindow::SaveAsset(CurrentGraph);
	return FReply::Handled();
}

FReply SSBF_SocialGraphTab::OnRevertClicked()
{
	if (CurrentGraph)
	{
		SetGraph(Cast<USBF_SocialGraph>(SSBF_EditorWindow::RevertAsset(CurrentGraph)));
	}
	return FReply::Handled();
}

FReply SSBF_SocialGraphTab::OnValidateClicked()
{
	ValidateSelection();
	return FReply::Handled();
}

FReply SSBF_SocialGraphTab::OnArrangeClicked()
{
	if (Canvas)
	{
		Canvas->ArrangeCircleLayout();
	}
	return FReply::Handled();
}

void SSBF_SocialGraphTab::SetGraph(USBF_SocialGraph* NewGraph)
{
	CurrentGraph = NewGraph;
	CurrentGraphPath = NewGraph ? NewGraph->GetPathName() : FString();
	RebuildCanvas();
}

void SSBF_SocialGraphTab::RebuildCanvas()
{
	if (Canvas)
	{
		Canvas->RebuildGraph(CurrentGraph);
	}
}

void SSBF_SocialGraphTab::RefreshSelection()
{
	if (CurrentGraphPath.IsEmpty())
	{
		return;
	}

	USBF_SocialGraph* Reloaded = LoadObject<USBF_SocialGraph>(nullptr, *CurrentGraphPath);
	SetGraph(Reloaded);
}

bool SSBF_SocialGraphTab::ValidateSelection()
{
	if (!CurrentGraph)
	{
		ValidationText->SetError(LOCTEXT("NoGraphSelected", "No Social Graph selected."));
		return false;
	}

	TArray<FText> Errors;
	const bool bValid = CurrentGraph->Validate(Errors);
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

void SSBF_SocialGraphTab::HandleNodeContextMenu(FGuid NodeGuid, FVector2D ScreenPosition)
{
	FSlateApplication::Get().PushMenu(
		AsShared(),
		FWidgetPath(),
		BuildNodeMenu(NodeGuid),
		ScreenPosition,
		FPopupTransitionEffect(EPopupTransitionEffect::ContextMenu));
}

void SSBF_SocialGraphTab::HandleCanvasContextMenu(FVector2D ScreenPosition)
{
	FSlateApplication::Get().PushMenu(
		AsShared(),
		FWidgetPath(),
		BuildCanvasMenu(),
		ScreenPosition,
		FPopupTransitionEffect(EPopupTransitionEffect::ContextMenu));
}

TSharedRef<SWidget> SSBF_SocialGraphTab::BuildNodeMenu(FGuid NodeGuid)
{
	const bool bCloseAfterSelection = true;
	FMenuBuilder MenuBuilder(bCloseAfterSelection, nullptr);

	MenuBuilder.BeginSection(TEXT("NodeActions"), LOCTEXT("NodeActionsSection", "Node"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("RenameNode", "Rename..."), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::RenameNode, NodeGuid)));
		MenuBuilder.AddMenuEntry(
			LOCTEXT("BindToSelection", "Bind to Selected Actor"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::BindNodeToSelection, NodeGuid)));
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteNode", "Delete"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::DeleteNode, NodeGuid)));
	}
	MenuBuilder.EndSection();

	if (CurrentGraph)
	{
		MenuBuilder.BeginSection(TEXT("AssignRelation"), LOCTEXT("AssignRelationSection", "Relation"));
		{
			for (const TObjectPtr<USBF_SocialRelation>& Relation : CurrentGraph->RelationAssets)
			{
				if (!Relation)
				{
					continue;
				}
				MenuBuilder.AddMenuEntry(
					Relation->GetRelationDisplayName(), FText::GetEmpty(), FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::AssignRelationToNode, NodeGuid, Relation.Get())));
			}
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ClearRelation", "(None)"), FText::GetEmpty(), FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::AssignRelationToNode, NodeGuid, static_cast<USBF_SocialRelation*>(nullptr))));
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection(TEXT("Parent"), LOCTEXT("ParentSection", "Set Parent"));
		{
			for (const FSBF_SocialNode& Node : CurrentGraph->Nodes)
			{
				if (Node.NodeGuid == NodeGuid)
				{
					continue;
				}
				MenuBuilder.AddMenuEntry(
					FText::FromName(Node.DisplayName), FText::GetEmpty(), FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::SetNodeParent, NodeGuid, Node.NodeGuid)));
			}
			MenuBuilder.AddMenuEntry(
				LOCTEXT("Unparent", "(Unparent)"), FText::GetEmpty(), FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::SetNodeParent, NodeGuid, FGuid())));
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection(TEXT("Edges"), LOCTEXT("EdgesSection", "Create Edge To"));
		{
			for (const FSBF_SocialNode& Node : CurrentGraph->Nodes)
			{
				if (Node.NodeGuid == NodeGuid)
				{
					continue;
				}
				MenuBuilder.AddMenuEntry(
					FText::FromName(Node.DisplayName), FText::GetEmpty(), FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::CreateEdge, NodeGuid, Node.NodeGuid)));
			}
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SSBF_SocialGraphTab::BuildCanvasMenu()
{
	const bool bCloseAfterSelection = true;
	FMenuBuilder MenuBuilder(bCloseAfterSelection, nullptr);

	MenuBuilder.BeginSection(TEXT("Canvas"), LOCTEXT("CanvasSection", "Social Graph"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AddNode", "Add NPC Node"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::AddNode)));
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ArrangeLayout", "Arrange Layout"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SSBF_SocialGraphTab::OnArrangeClicked)));
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ClearGraph", "Clear Graph"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				if (CurrentGraph)
				{
					CurrentGraph->ClearGraph();
					RebuildCanvas();
				}
			})));
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SSBF_SocialGraphTab::AddNode()
{
	if (!CurrentGraph)
	{
		ValidationText->SetError(LOCTEXT("NoGraphForNode", "Select or create a Social Graph first."));
		return;
	}

	// Bind to the first selected actor when available (nice for quick setup).
	FGuid ActorGuid;
	if (GEditor)
	{
		if (USelection* Selection = GEditor->GetSelectedActors())
		{
			if (Selection->Num() > 0)
			{
				if (const AActor* Actor = Cast<AActor>(Selection->GetSelectedObject(0)))
				{
					ActorGuid = Actor->GetActorGuid();
				}
			}
		}
	}

	const FGuid NewGuid = CurrentGraph->AddNode(
		ActorGuid,
		FName(*FString::Printf(TEXT("NPC_%d"), CurrentGraph->Nodes.Num() + 1)));

	if (NewGuid.IsValid())
	{
		RebuildCanvas();
	}
}

void SSBF_SocialGraphTab::DeleteNode(FGuid NodeGuid)
{
	if (CurrentGraph)
	{
		CurrentGraph->RemoveNode(NodeGuid);
		RebuildCanvas();
	}
}

void SSBF_SocialGraphTab::AssignRelationToNode(FGuid NodeGuid, USBF_SocialRelation* Relation)
{
	if (!CurrentGraph)
	{
		return;
	}

	FSBF_SocialNode Node;
	if (!CurrentGraph->FindNode(NodeGuid, Node))
	{
		return;
	}

	const FGameplayTag NewTag = Relation ? Relation->RelationTag : FGameplayTag();
	CurrentGraph->SetParent(NodeGuid, Node.ParentGuid, NewTag);
	RebuildCanvas();
}

void SSBF_SocialGraphTab::SetNodeParent(FGuid NodeGuid, FGuid NewParentGuid)
{
	if (!CurrentGraph)
	{
		return;
	}

	FSBF_SocialNode Node;
	if (!CurrentGraph->FindNode(NodeGuid, Node))
	{
		return;
	}

	FGameplayTag Tag = Node.RelationTag;
	if (!NewParentGuid.IsValid() && Node.ParentGuid.IsValid())
	{
		// Unparenting keeps the old relation tag unless the user picked a parent.
		Tag = FGameplayTag();
	}

	if (CurrentGraph->SetParent(NodeGuid, NewParentGuid, Tag))
	{
		RebuildCanvas();
	}
}

void SSBF_SocialGraphTab::CreateEdge(FGuid NodeA, FGuid NodeB)
{
	if (!CurrentGraph)
	{
		return;
	}

	// Default: the source node's relation asset (or the first relation asset).
	USBF_SocialRelation* Relation = nullptr;
	FSBF_SocialNode NodeAInfo;
	if (CurrentGraph->FindNode(NodeA, NodeAInfo))
	{
		Relation = const_cast<USBF_SocialRelation*>(CurrentGraph->FindRelationAsset(NodeAInfo.RelationTag));
	}
	if (!Relation && CurrentGraph->RelationAssets.Num() > 0)
	{
		Relation = CurrentGraph->RelationAssets[0];
	}

	const FGuid EdgeGuid = CurrentGraph->Link(NodeA, NodeB, Relation, /*bBidirectional=*/true);
	if (EdgeGuid.IsValid())
	{
		RebuildCanvas();
	}
}

void SSBF_SocialGraphTab::BindNodeToSelection(FGuid NodeGuid)
{
	if (!CurrentGraph || !GEditor)
	{
		return;
	}

	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection || Selection->Num() == 0)
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: select an actor in the level first."));
		return;
	}

	const AActor* Actor = Cast<AActor>(Selection->GetSelectedObject(0));
	if (!Actor)
	{
		return;
	}

	if (CurrentGraph->BindNodeToActor(NodeGuid, Actor->GetActorGuid()))
	{
		RebuildCanvas();
	}
}

void SSBF_SocialGraphTab::RenameNode(FGuid NodeGuid)
{
	if (!CurrentGraph)
	{
		return;
	}

	FSBF_SocialNode Node;
	if (!CurrentGraph->FindNode(NodeGuid, Node))
	{
		return;
	}

	FText NewName;
	if (PromptForText(LOCTEXT("RenameDialogTitle", "Rename Node"), FText::FromName(Node.DisplayName), NewName))
	{
		CurrentGraph->RenameNode(NodeGuid, FName(*NewName.ToString()));
		RebuildCanvas();
	}
}

bool SSBF_SocialGraphTab::PromptForText(const FText& Title, const FText& InitialValue, FText& OutValue) const
{
	bool bConfirmed = false;
	TSharedPtr<SEditableTextBox> TextBox;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(Title)
		.ClientSize(FVector2D(320.0f, 96.0f))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::FixedSize)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 8.0f, 8.0f, 0.0f)
			[
				SAssignNew(TextBox, SEditableTextBox)
				.Text(InitialValue)
				.SelectAllTextWhenFocused(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 0.0f, 8.0f, 8.0f)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("RenameOK", "OK"))
					.OnClicked_Lambda([&bConfirmed, &OutValue, TextBox, Window]()
					{
						bConfirmed = true;
						OutValue = TextBox->GetText();
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("RenameCancel", "Cancel"))
					.OnClicked_Lambda([Window]()
					{
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
			]
		];

	FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().GetActiveTopLevelWindow());
	return bConfirmed;
}

FLinearColor SSBF_SocialGraphTab::GetHostilityColor(ESBF_Hostility Hostility)
{
	switch (Hostility)
	{
	case ESBF_Hostility::Enemy:
		return FLinearColor(0.95f, 0.25f, 0.25f);
	case ESBF_Hostility::Ally:
		return FLinearColor(0.25f, 0.85f, 0.30f);
	case ESBF_Hostility::Neutral:
		return FLinearColor(0.72f, 0.72f, 0.72f);
	default:
		return FLinearColor(0.35f, 0.35f, 0.35f);
	}
}

#undef LOCTEXT_NAMESPACE
