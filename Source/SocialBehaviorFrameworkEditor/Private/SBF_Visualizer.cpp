// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_Visualizer.h"

#include "DrawDebugHelpers.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "SBF_Log.h"
#include "SBF_NPCComponent.h"
#include "SBF_SocialGraph.h"

FSBF_Visualizer& FSBF_Visualizer::Get()
{
	static FSBF_Visualizer Visualizer;
	return Visualizer;
}

void FSBF_Visualizer::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	UE_LOG(LogSBF, Log, TEXT("SBF visualizer %s."), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

TStatId FSBF_Visualizer::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FSBF_Visualizer, STATGROUP_Tickables);
}

void FSBF_Visualizer::Tick(float DeltaTime)
{
	if (!bEnabled || !GEditor)
	{
		return;
	}

	DrawGraphEdges(GEditor->GetEditorWorldContext().World(), /*bIsPIE=*/false);

	if (const FWorldContext* PIEContext = GEditor->GetPIEWorldContext())
	{
		if (UWorld* PIEWorld = PIEContext->World())
		{
			DrawGraphEdges(PIEWorld, /*bIsPIE=*/true);
		}
	}
}

void FSBF_Visualizer::DrawGraphEdges(UWorld* World, bool bIsPIE) const
{
	if (!IsValid(World))
	{
		return;
	}

	const bool bDrawableWorld = bIsPIE || World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview;
	if (!bDrawableWorld)
	{
		return;
	}

	// Collect NPCs grouped by their social graph asset.
	TMap<const USBF_SocialGraph*, TMap<FGuid, FVector>> GraphToActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		const AActor* Actor = *It;
		const USBF_NPCComponent* Component = Actor->FindComponentByClass<USBF_NPCComponent>();
		if (!Component || !Component->GetSocialGraph())
		{
			continue;
		}
		GraphToActors.FindOrAdd(Component->GetSocialGraph()).Add(Actor->GetActorGuid(), Actor->GetActorLocation());
	}

	for (const TPair<const USBF_SocialGraph*, TMap<FGuid, FVector>>& Pair : GraphToActors)
	{
		const USBF_SocialGraph* Graph = Pair.Key;
		const TMap<FGuid, FVector>& ActorLocations = Pair.Value;
		if (ActorLocations.Num() < 2)
		{
			continue;
		}

		// Hierarchy edges (node -> parent).
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
			const FVector* From = ActorLocations.Find(Node.ActorGuid);
			const FVector* To = ActorLocations.Find(Parent.ActorGuid);
			if (!From || !To)
			{
				continue;
			}

			const ESBF_Hostility Hostility = Graph->ResolveHostility(Node.NodeGuid, Parent.NodeGuid);
			const FColor Color = Hostility == ESBF_Hostility::Enemy ? FColor(220, 60, 60) : (Hostility == ESBF_Hostility::Ally ? FColor(60, 220, 80) : FColor(190, 190, 190));

			if (bIsPIE)
			{
				DrawDebugLine(World, *From, *To, Color, /*bPersistentLines=*/false, /*LifeTime=*/-1.0f, /*DepthPriority=*/0, /*Thickness=*/2.0f);
			}
			World->GetPersistentLineBatcher()->DrawLine(*From, *To, Color, SDPG_World, 2.0f, 0.033f);
		}

		// Explicit edges.
		for (const TPair<FGuid, FSBF_SocialEdge>& EdgePair : Graph->Edges)
		{
			const FSBF_SocialEdge& Edge = EdgePair.Value;
			FSBF_SocialNode NodeA, NodeB;
			if (!Graph->FindNode(Edge.NodeA, NodeA) || !Graph->FindNode(Edge.NodeB, NodeB))
			{
				continue;
			}
			const FVector* From = ActorLocations.Find(NodeA.ActorGuid);
			const FVector* To = ActorLocations.Find(NodeB.ActorGuid);
			if (!From || !To)
			{
				continue;
			}

			const ESBF_Hostility Hostility = Graph->ResolveHostility(Edge.NodeA, Edge.NodeB);
			const FColor Color = Hostility == ESBF_Hostility::Enemy ? FColor(255, 40, 40) : (Hostility == ESBF_Hostility::Ally ? FColor(40, 255, 80) : FColor(220, 220, 220));

			if (bIsPIE)
			{
				DrawDebugLine(World, *From, *To, Color, /*bPersistentLines=*/false, /*LifeTime=*/-1.0f, /*DepthPriority=*/0, /*Thickness=*/4.0f);
			}
			World->GetPersistentLineBatcher()->DrawLine(*From, *To, Color, SDPG_World, 4.0f, 0.033f);
		}
	}
}
