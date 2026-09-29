// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_SocialGraph.h"

#include "Containers/Queue.h"
#include "SBF_Log.h"
#include "SBF_SocialRelation.h"
#include "UObject/Package.h"

namespace
{
	/** Normalizes a pair of guids so (A,B) and (B,A) share one cache entry. */
	TPair<FGuid, FGuid> NormalizePair(const FGuid& A, const FGuid& B)
	{
		return (A < B) ? TPair<FGuid, FGuid>(A, B) : TPair<FGuid, FGuid>(B, A);
	}
}

FGuid USBF_SocialGraph::AddNode(FGuid ActorGuid, FName DisplayName, FGuid ParentGuid, FGameplayTag RelationTag)
{
	FSBF_SocialNode NewNode;
	NewNode.NodeGuid = FGuid::NewGuid();
	NewNode.ActorGuid = ActorGuid;
	NewNode.DisplayName = DisplayName;
	NewNode.ParentGuid = ParentGuid;
	NewNode.RelationTag = RelationTag;

	Nodes.Add(NewNode);
	MarkDirty();

	UE_LOG(LogSBF, Verbose, TEXT("SocialGraph '%s': added node '%s' (%s)"), *GetName(), *DisplayName.ToString(), *NewNode.NodeGuid.ToString());
	return NewNode.NodeGuid;
}

bool USBF_SocialGraph::RemoveNode(FGuid NodeGuid)
{
	const int32 Index = Nodes.IndexOfByPredicate([NodeGuid](const FSBF_SocialNode& Node) { return Node.NodeGuid == NodeGuid; });
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogSBF, Warning, TEXT("SocialGraph '%s': RemoveNode failed, node %s not found"), *GetName(), *NodeGuid.ToString());
		return false;
	}

	const FGuid ParentGuid = Nodes[Index].ParentGuid;

	// Reparent children to the removed node's parent.
	for (FSBF_SocialNode& Node : Nodes)
	{
		if (Node.ParentGuid == NodeGuid)
		{
			Node.ParentGuid = ParentGuid;
		}
	}

	// Drop touching edges.
	TArray<FGuid> EdgesToRemove;
	for (const TPair<FGuid, FSBF_SocialEdge>& Pair : Edges)
	{
		if (Pair.Value.NodeA == NodeGuid || Pair.Value.NodeB == NodeGuid)
		{
			EdgesToRemove.Add(Pair.Key);
		}
	}
	for (const FGuid& EdgeGuid : EdgesToRemove)
	{
		Edges.Remove(EdgeGuid);
	}

	Nodes.RemoveAt(Index);
	MarkDirty();
	return true;
}

bool USBF_SocialGraph::BindNodeToActor(FGuid NodeGuid, FGuid ActorGuid)
{
	FSBF_SocialNode* Node = Nodes.FindByPredicate([NodeGuid](const FSBF_SocialNode& InNode) { return InNode.NodeGuid == NodeGuid; });
	if (!Node)
	{
		UE_LOG(LogSBF, Warning, TEXT("SocialGraph '%s': BindNodeToActor failed, node %s not found"), *GetName(), *NodeGuid.ToString());
		return false;
	}
	if (Node->ActorGuid == ActorGuid)
	{
		return false;
	}

	Node->ActorGuid = ActorGuid;
	MarkDirty();
	return true;
}

bool USBF_SocialGraph::RenameNode(FGuid NodeGuid, FName NewDisplayName)
{
	FSBF_SocialNode* Node = Nodes.FindByPredicate([NodeGuid](const FSBF_SocialNode& InNode) { return InNode.NodeGuid == NodeGuid; });
	if (!Node || Node->DisplayName == NewDisplayName)
	{
		return false;
	}

	Node->DisplayName = NewDisplayName;
	MarkDirty();
	return true;
}

void USBF_SocialGraph::ClearGraph()
{
	Nodes.Empty();
	Edges.Empty();
	MarkDirty();
	UE_LOG(LogSBF, Log, TEXT("SocialGraph '%s' cleared."), *GetName());
}

FGuid USBF_SocialGraph::Link(FGuid NodeA, FGuid NodeB, USBF_SocialRelation* Relation, bool bBidirectional)
{
	FSBF_SocialNode NodeAInfo, NodeBInfo;
	if (!FindNode(NodeA, NodeAInfo) || !FindNode(NodeB, NodeBInfo))
	{
		UE_LOG(LogSBF, Warning, TEXT("SocialGraph '%s': Link failed, node not found (A=%s, B=%s)"), *GetName(), *NodeA.ToString(), *NodeB.ToString());
		return FGuid();
	}

	FSBF_SocialEdge Edge;
	Edge.EdgeGuid = FGuid::NewGuid();
	Edge.NodeA = NodeA;
	Edge.NodeB = NodeB;
	Edge.Relation = Relation;
	Edge.RelationTag = Relation ? Relation->RelationTag : FGameplayTag();
	Edge.bBidirectional = bBidirectional;

	Edges.Add(Edge.EdgeGuid, Edge);
	MarkDirty();
	return Edge.EdgeGuid;
}

bool USBF_SocialGraph::Unlink(FGuid EdgeGuid)
{
	if (Edges.Remove(EdgeGuid) == 0)
	{
		UE_LOG(LogSBF, Warning, TEXT("SocialGraph '%s': Unlink failed, edge %s not found"), *GetName(), *EdgeGuid.ToString());
		return false;
	}
	MarkDirty();
	return true;
}

bool USBF_SocialGraph::SetParent(FGuid NodeGuid, FGuid NewParentGuid, FGameplayTag RelationTag)
{
	FSBF_SocialNode* Node = Nodes.FindByPredicate([NodeGuid](const FSBF_SocialNode& InNode) { return InNode.NodeGuid == NodeGuid; });
	if (!Node)
	{
		UE_LOG(LogSBF, Warning, TEXT("SocialGraph '%s': SetParent failed, node %s not found"), *GetName(), *NodeGuid.ToString());
		return false;
	}

	if (Node->ParentGuid == NewParentGuid && Node->RelationTag == RelationTag)
	{
		return false;
	}

	// Reject direct cycles (parent of itself).
	if (NewParentGuid == NodeGuid)
	{
		UE_LOG(LogSBF, Warning, TEXT("SocialGraph '%s': SetParent rejected, node cannot be its own parent"), *GetName());
		return false;
	}

	// Reject ancestor cycles (walking up from the new parent must not reach the node).
	FGuid Walk = NewParentGuid;
	TSet<FGuid> Visited;
	while (Walk.IsValid() && !Visited.Contains(Walk))
	{
		if (Walk == NodeGuid)
		{
			UE_LOG(LogSBF, Warning, TEXT("SocialGraph '%s': SetParent rejected, the new parent is a descendant of the node"), *GetName());
			return false;
		}
		Visited.Add(Walk);
		FSBF_SocialNode WalkNode;
		if (!FindNode(Walk, WalkNode))
		{
			break;
		}
		Walk = WalkNode.ParentGuid;
	}

	Node->ParentGuid = NewParentGuid;
	Node->RelationTag = RelationTag;
	MarkDirty();
	return true;
}

ESBF_Hostility USBF_SocialGraph::ResolveHostility(FGuid NodeA, FGuid NodeB) const
{
	if (!NodeA.IsValid() || !NodeB.IsValid())
	{
		return ESBF_Hostility::Unknown;
	}
	if (NodeA == NodeB)
	{
		return ESBF_Hostility::Neutral;
	}

	const TPair<FGuid, FGuid> Key = NormalizePair(NodeA, NodeB);
	if (const uint8* Cached = HostilityCache.Find(Key))
	{
		return static_cast<ESBF_Hostility>(*Cached);
	}

	const ESBF_Hostility Result = ResolveHostilityInternal(NodeA, NodeB);
	HostilityCache.Add(Key, static_cast<uint8>(Result));
	return Result;
}

ESBF_Hostility USBF_SocialGraph::ResolveHostilityForActors(FGuid ActorA, FGuid ActorB) const
{
	FSBF_SocialNode NodeA, NodeB;
	if (!FindNodeByActor(ActorA, NodeA) || !FindNodeByActor(ActorB, NodeB))
	{
		return ESBF_Hostility::Unknown;
	}
	return ResolveHostility(NodeA.NodeGuid, NodeB.NodeGuid);
}

TArray<FGuid> USBF_SocialGraph::GetAlliesOf(FGuid NodeGuid) const
{
	TArray<FGuid> Allies;
	for (const FSBF_SocialNode& Node : Nodes)
	{
		if (Node.NodeGuid != NodeGuid && ResolveHostility(NodeGuid, Node.NodeGuid) == ESBF_Hostility::Ally)
		{
			Allies.Add(Node.NodeGuid);
		}
	}
	return Allies;
}

TArray<FGuid> USBF_SocialGraph::GetEnemiesOf(FGuid NodeGuid) const
{
	TArray<FGuid> Enemies;
	for (const FSBF_SocialNode& Node : Nodes)
	{
		if (Node.NodeGuid != NodeGuid && ResolveHostility(NodeGuid, Node.NodeGuid) == ESBF_Hostility::Enemy)
		{
			Enemies.Add(Node.NodeGuid);
		}
	}
	return Enemies;
}

TArray<FGuid> USBF_SocialGraph::GetChildrenOf(FGuid NodeGuid) const
{
	TArray<FGuid> Children;
	for (const FSBF_SocialNode& Node : Nodes)
	{
		if (Node.ParentGuid == NodeGuid)
		{
			Children.Add(Node.NodeGuid);
		}
	}
	return Children;
}

bool USBF_SocialGraph::GetParentOf(FGuid NodeGuid, FSBF_SocialNode& OutParent) const
{
	for (const FSBF_SocialNode& Node : Nodes)
	{
		if (Node.NodeGuid == NodeGuid)
		{
			return FindNode(Node.ParentGuid, OutParent);
		}
	}
	return false;
}

bool USBF_SocialGraph::FindNodeByActor(FGuid ActorGuid, FSBF_SocialNode& OutNode) const
{
	const FSBF_SocialNode* Found = Nodes.FindByPredicate([ActorGuid](const FSBF_SocialNode& Node) { return Node.ActorGuid == ActorGuid; });
	if (Found)
	{
		OutNode = *Found;
		return true;
	}
	return false;
}

bool USBF_SocialGraph::FindNode(FGuid NodeGuid, FSBF_SocialNode& OutNode) const
{
	const FSBF_SocialNode* Found = Nodes.FindByPredicate([NodeGuid](const FSBF_SocialNode& Node) { return Node.NodeGuid == NodeGuid; });
	if (Found)
	{
		OutNode = *Found;
		return true;
	}
	return false;
}

const USBF_SocialRelation* USBF_SocialGraph::FindRelationAsset(FGameplayTag RelationTag) const
{
	for (const TObjectPtr<USBF_SocialRelation>& Relation : RelationAssets)
	{
		if (Relation && Relation->RelationTag == RelationTag)
		{
			return Relation;
		}
	}
	return nullptr;
}

FGameplayTag USBF_SocialGraph::GetDirectRelationTag(FGuid NodeA, FGuid NodeB) const
{
	// Explicit edge wins.
	for (const TPair<FGuid, FSBF_SocialEdge>& Pair : Edges)
	{
		const FSBF_SocialEdge& Edge = Pair.Value;
		if ((Edge.NodeA == NodeA && Edge.NodeB == NodeB) || (Edge.bBidirectional && Edge.NodeA == NodeB && Edge.NodeB == NodeA))
		{
			return Edge.RelationTag;
		}
	}

	// Hierarchy: either node is the parent of the other.
	for (const FSBF_SocialNode& Node : Nodes)
	{
		if (Node.NodeGuid == NodeA && Node.ParentGuid == NodeB)
		{
			return Node.RelationTag;
		}
		if (Node.NodeGuid == NodeB && Node.ParentGuid == NodeA)
		{
			return Node.RelationTag;
		}
	}
	return FGameplayTag();
}

void USBF_SocialGraph::InvalidateHostilityCache() const
{
	HostilityCache.Empty();
}

bool USBF_SocialGraph::Validate(TArray<FText>& OutErrors) const
{
	bool bValid = true;

	TSet<FGuid> NodeGuids;
	TSet<FGuid> ActorGuids;
	for (const FSBF_SocialNode& Node : Nodes)
	{
		if (!Node.NodeGuid.IsValid())
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Node '%s' has an invalid node guid."), *Node.DisplayName.ToString())));
			bValid = false;
			continue;
		}
		if (!Node.ActorGuid.IsValid())
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Node '%s' has an invalid actor guid (not bound to an actor)."), *Node.DisplayName.ToString())));
			bValid = false;
		}
		if (ActorGuids.Contains(Node.ActorGuid))
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Duplicate actor guid '%s' on node '%s'."), *Node.ActorGuid.ToString(), *Node.DisplayName.ToString())));
			bValid = false;
		}
		NodeGuids.Add(Node.NodeGuid);
		ActorGuids.Add(Node.ActorGuid);
	}

	// Dangling parents + hierarchy cycles.
	for (const FSBF_SocialNode& Node : Nodes)
	{
		if (Node.ParentGuid.IsValid() && !NodeGuids.Contains(Node.ParentGuid))
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Node '%s' references a missing parent."), *Node.DisplayName.ToString())));
			bValid = false;
			continue;
		}

		// Walk the parent chain looking for a cycle.
		TSet<FGuid> Visited;
		FGuid Current = Node.ParentGuid;
		while (Current.IsValid())
		{
			if (Visited.Contains(Current))
			{
				OutErrors.Add(FText::FromString(FString::Printf(TEXT("Cycle detected in the parent chain of node '%s'."), *Node.DisplayName.ToString())));
				bValid = false;
				break;
			}
			if (Current == Node.NodeGuid)
			{
				OutErrors.Add(FText::FromString(FString::Printf(TEXT("Node '%s' is its own ancestor."), *Node.DisplayName.ToString())));
				bValid = false;
				break;
			}
			Visited.Add(Current);
			FSBF_SocialNode Parent;
			if (!FindNode(Current, Parent))
			{
				break;
			}
			Current = Parent.ParentGuid;
		}
	}

	// Edge integrity.
	for (const TPair<FGuid, FSBF_SocialEdge>& Pair : Edges)
	{
		const FSBF_SocialEdge& Edge = Pair.Value;
		if (!NodeGuids.Contains(Edge.NodeA) || !NodeGuids.Contains(Edge.NodeB))
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Edge '%s' references a missing node."), *Edge.EdgeGuid.ToString())));
			bValid = false;
		}
		if (Edge.Relation == nullptr && !Edge.RelationTag.IsValid())
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Edge '%s' has no relation assigned."), *Edge.EdgeGuid.ToString())));
			bValid = false;
		}
		if (Edge.NodeA == Edge.NodeB)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Edge '%s' connects a node to itself."), *Edge.EdgeGuid.ToString())));
			bValid = false;
		}
	}

	return bValid;
}

ESBF_Hostility USBF_SocialGraph::ResolveHostilityInternal(FGuid NodeA, FGuid NodeB) const
{
	// Direct explicit edge short-circuits the inference.
	for (const TPair<FGuid, FSBF_SocialEdge>& Pair : Edges)
	{
		const FSBF_SocialEdge& Edge = Pair.Value;
		const bool bConnects = (Edge.NodeA == NodeA && Edge.NodeB == NodeB) || (Edge.bBidirectional && Edge.NodeA == NodeB && Edge.NodeB == NodeA);
		if (bConnects)
		{
			FGraphHop Hop;
			Hop.From = NodeA;
			Hop.To = NodeB;
			Hop.RelationTag = Edge.RelationTag;
			Hop.Relation = Edge.Relation ? Edge.Relation : FindRelationAsset(Edge.RelationTag);
			return GetHopHostility(Hop);
		}
	}

	// Direct hierarchy edge.
	for (const FSBF_SocialNode& Node : Nodes)
	{
		const bool bChildOfB = Node.NodeGuid == NodeA && Node.ParentGuid == NodeB;
		const bool bParentOfB = Node.NodeGuid == NodeB && Node.ParentGuid == NodeA;
		if (bChildOfB || bParentOfB)
		{
			FGraphHop Hop;
			Hop.From = bChildOfB ? NodeA : NodeB;
			Hop.To = bChildOfB ? NodeB : NodeA;
			Hop.RelationTag = Node.RelationTag;
			Hop.Relation = FindRelationAsset(Node.RelationTag);
			return GetHopHostility(Hop);
		}
	}

	// No direct connection: walk the shortest path and combine hops.
	TArray<FGraphHop> Path;
	if (!FindShortestPath(NodeA, NodeB, Path) || Path.Num() == 0)
	{
		return ESBF_Hostility::Unknown;
	}

	// Hostility propagates up the chain unconditionally (nearest common
	// ancestor rule). Ally stance propagates only along an unbroken chain of
	// relations with bInheritsHostility (direct relations always apply).
	ESBF_Hostility Result = ESBF_Hostility::Neutral;
	bool bChainInherits = true;
	for (int32 HopIndex = 0; HopIndex < Path.Num(); ++HopIndex)
	{
		const FGraphHop& Hop = Path[HopIndex];
		const ESBF_Hostility HopStance = GetHopHostility(Hop);

		if (HopStance == ESBF_Hostility::Enemy)
		{
			return ESBF_Hostility::Enemy;
		}

		if (HopStance == ESBF_Hostility::Ally)
		{
			const bool bHopInherits = (Hop.Relation == nullptr || Hop.Relation->bInheritsHostility);
			if (HopIndex == 0 || (bChainInherits && bHopInherits))
			{
				// "Ally of my father is my ally" - requires every hop of the
				// chain (including the father hop) to inherit.
				Result = ESBF_Hostility::Ally;
			}
		}

		bChainInherits = bChainInherits && (Hop.Relation == nullptr || Hop.Relation->bInheritsHostility);
	}
	return Result;
}

bool USBF_SocialGraph::FindShortestPath(FGuid NodeA, FGuid NodeB, TArray<FGraphHop>& OutPath) const
{
	TMap<FGuid, FGraphHop> CameFrom;
	TQueue<FGuid> Queue;
	TSet<FGuid> Visited;

	Queue.Enqueue(NodeA);
	Visited.Add(NodeA);

	while (!Queue.IsEmpty())
	{
		FGuid Current;
		Queue.Dequeue(Current);
		if (Current == NodeB)
		{
			break;
		}

		TArray<FGraphHop> Neighbors;
		CollectNeighbors(Current, Neighbors);
		for (const FGraphHop& Hop : Neighbors)
		{
			if (!Visited.Contains(Hop.To))
			{
				Visited.Add(Hop.To);
				CameFrom.Add(Hop.To, Hop);
				Queue.Enqueue(Hop.To);
			}
		}
	}

	if (!CameFrom.Contains(NodeB) && NodeA != NodeB)
	{
		return false;
	}

	FGuid Back = NodeB;
	while (Back != NodeA)
	{
		const FGraphHop* Hop = CameFrom.Find(Back);
		if (!Hop)
		{
			OutPath.Empty();
			return false;
		}
		OutPath.Insert(*Hop, 0);
		Back = Hop->From;
	}
	return true;
}

void USBF_SocialGraph::CollectNeighbors(FGuid NodeGuid, TArray<FGraphHop>& OutHops) const
{
	FSBF_SocialNode Node;
	if (!FindNode(NodeGuid, Node))
	{
		return;
	}

	// Hierarchy: parent.
	if (Node.ParentGuid.IsValid())
	{
		FGraphHop Hop;
		Hop.From = NodeGuid;
		Hop.To = Node.ParentGuid;
		Hop.RelationTag = Node.RelationTag;
		Hop.Relation = FindRelationAsset(Node.RelationTag);
		OutHops.Add(Hop);
	}

	// Hierarchy: children.
	for (const FSBF_SocialNode& Child : Nodes)
	{
		if (Child.ParentGuid == NodeGuid)
		{
			FGraphHop Hop;
			Hop.From = NodeGuid;
			Hop.To = Child.NodeGuid;
			Hop.RelationTag = Child.RelationTag;
			Hop.Relation = FindRelationAsset(Child.RelationTag);
			OutHops.Add(Hop);
		}
	}

	// Explicit edges.
	for (const TPair<FGuid, FSBF_SocialEdge>& Pair : Edges)
	{
		const FSBF_SocialEdge& Edge = Pair.Value;
		FGuid Other;
		if (Edge.NodeA == NodeGuid)
		{
			Other = Edge.NodeB;
		}
		else if (Edge.NodeB == NodeGuid && Edge.bBidirectional)
		{
			Other = Edge.NodeA;
		}
		else
		{
			continue;
		}

		FGraphHop Hop;
		Hop.From = NodeGuid;
		Hop.To = Other;
		Hop.RelationTag = Edge.RelationTag;
		Hop.Relation = Edge.Relation ? Edge.Relation : FindRelationAsset(Edge.RelationTag);
		OutHops.Add(Hop);
	}
}

ESBF_Hostility USBF_SocialGraph::GetHopHostility(const FGraphHop& Hop) const
{
	if (Hop.Relation)
	{
		// Prefer the explicit relation asset; if it lists a stance for the hop
		// tag, honor it, otherwise fall back to its base stance.
		const ESBF_Hostility TowardsTag = Hop.Relation->GetHostilityTowards(Hop.RelationTag);
		if (TowardsTag != ESBF_Hostility::Neutral)
		{
			return TowardsTag;
		}
		return Hop.Relation->GetBaseHostility();
	}

	const USBF_SocialRelation* Asset = FindRelationAsset(Hop.RelationTag);
	return Asset ? Asset->GetBaseHostility() : ESBF_Hostility::Neutral;
}

void USBF_SocialGraph::MarkDirty()
{
	InvalidateHostilityCache();
#if WITH_EDITOR
	MarkPackageDirty();
#endif
}

#if WITH_EDITOR
void USBF_SocialGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	InvalidateHostilityCache();
}
#endif

void USBF_SocialGraph::PostLoad()
{
	Super::PostLoad();
	InvalidateHostilityCache();
}
