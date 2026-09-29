// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBF_Types.h"
#include "SBF_SocialGraph.generated.h"

class USBF_SocialRelation;

/**
 * Social Graph data asset: a set of NPC nodes (hierarchy) plus explicit
 * relation edges.
 *
 * Hostility resolution rules (see ResolveHostility):
 *  - a direct edge between two nodes short-circuits the query;
 *  - otherwise the shortest path through hierarchy + explicit edges is walked
 *    and every hostile hop marks the pair as hostile (hostility propagates up
 *    to the nearest common ancestor);
 *  - ally hops only propagate when the hop relation has bInheritsHostility.
 *
 * Results are cached in TMap<TPair<FGuid,FGuid>, uint8> (keyed by node guids)
 * and invalidated on every structural mutation or editor change.
 */
UCLASS(BlueprintType, hidecategories = (Object))
class SOCIALBEHAVIORFRAMEWORK_API USBF_SocialGraph : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Relation types available in this graph (used for edge/parent assignment). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social Graph")
	TArray<TObjectPtr<USBF_SocialRelation>> RelationAssets;

	/** All graph nodes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social Graph")
	TArray<FSBF_SocialNode> Nodes;

	/** Explicit edges, keyed by edge guid. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social Graph")
	TMap<FGuid, FSBF_SocialEdge> Edges;

	/**
	 * Adds a new node to the graph.
	 * @param ActorGuid guid of the world actor this node represents.
	 * @param DisplayName editor display name.
	 * @param ParentGuid optional hierarchy parent.
	 * @param RelationTag relation of the node towards its parent.
	 * @return the new node guid (empty guid on failure).
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	FGuid AddNode(FGuid ActorGuid, FName DisplayName, FGuid ParentGuid = FGuid(), FGameplayTag RelationTag = FGameplayTag());

	/**
	 * Removes a node and cascades: children are re-parented to the removed
	 * node's parent and all touching edges are deleted.
	 * @return true when the node existed.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool RemoveNode(FGuid NodeGuid);

	/**
	 * Creates an explicit edge between two nodes.
	 * @return the new edge guid (empty on failure).
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	FGuid Link(FGuid NodeA, FGuid NodeB, USBF_SocialRelation* Relation = nullptr, bool bBidirectional = true);

	/** Removes an explicit edge by guid. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool Unlink(FGuid EdgeGuid);

	/**
	 * Reparents a node.
	 * @return true when the node exists and a change was applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool SetParent(FGuid NodeGuid, FGuid NewParentGuid, FGameplayTag RelationTag = FGameplayTag());

	/**
	 * Binds a node to a world actor guid (used by the editor window's
	 * "Bind to Selected Actor" action).
	 * @return true when the node exists and the binding changed.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool BindNodeToActor(FGuid NodeGuid, FGuid ActorGuid);

	/** Renames a node's display name. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool RenameNode(FGuid NodeGuid, FName NewDisplayName);

	/** Removes every node and edge (used by the editor's "Clear Graph" action). */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void ClearGraph();

	/**
	 * Resolves the hostility between two nodes (see class comment for rules).
	 * Cached; safe to call every frame.
	 */
	UFUNCTION(BlueprintPure, Category = "SBF")
	ESBF_Hostility ResolveHostility(FGuid NodeA, FGuid NodeB) const;

	/** Convenience wrapper resolving by actor guids. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	ESBF_Hostility ResolveHostilityForActors(FGuid ActorA, FGuid ActorB) const;

	/** @return all nodes allied with the given node. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	TArray<FGuid> GetAlliesOf(FGuid NodeGuid) const;

	/** @return all nodes hostile to the given node. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	TArray<FGuid> GetEnemiesOf(FGuid NodeGuid) const;

	/** @return guids of the direct hierarchy children of the node. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	TArray<FGuid> GetChildrenOf(FGuid NodeGuid) const;

	/** @return true and the parent node when it exists. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool GetParentOf(FGuid NodeGuid, FSBF_SocialNode& OutParent) const;

	/** @return true and the node bound to the given actor guid. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool FindNodeByActor(FGuid ActorGuid, FSBF_SocialNode& OutNode) const;

	/** @return true and the node with the given node guid. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool FindNode(FGuid NodeGuid, FSBF_SocialNode& OutNode) const;

	/** @return the relation asset with the given identity tag (nullptr when absent). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	const USBF_SocialRelation* FindRelationAsset(FGameplayTag RelationTag) const;

	/**
	 * @return the direct relation tag between two node guids
	 *         (explicit edge first, then hierarchy); invalid tag when unrelated.
	 */
	UFUNCTION(BlueprintPure, Category = "SBF")
	FGameplayTag GetDirectRelationTag(FGuid NodeA, FGuid NodeB) const;

	/** Drops the hostility cache. Called automatically by every mutation. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void InvalidateHostilityCache() const;

	/**
	 * Structural validation used by the editor window.
	 * @param OutErrors appended localized error descriptions.
	 * @return true when the graph is structurally valid.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool Validate(TArray<FText>& OutErrors) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;

private:
	/** Shortest-path hop between two adjacent nodes. */
	struct FGraphHop
	{
		FGuid From;
		FGuid To;
		FGameplayTag RelationTag;
		const USBF_SocialRelation* Relation = nullptr;
	};

	/** Hostility cache keyed by normalized node guid pair. */
	mutable UPROPERTY(Transient)
	TMap<TPair<FGuid, FGuid>, uint8> HostilityCache;

	ESBF_Hostility ResolveHostilityInternal(FGuid NodeA, FGuid NodeB) const;
	bool FindShortestPath(FGuid NodeA, FGuid NodeB, TArray<FGraphHop>& OutPath) const;
	void CollectNeighbors(FGuid NodeGuid, TArray<FGraphHop>& OutHops) const;
	ESBF_Hostility GetHopHostility(const FGraphHop& Hop) const;
	void MarkDirty();
};
