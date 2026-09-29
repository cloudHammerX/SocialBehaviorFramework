// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBF_Types.h"
#include "SBF_SocialRelation.generated.h"

class UUserWidget;

/**
 * Data asset describing a single relation type of the social hierarchy
 * ("father", "boss", "rival" ...).
 *
 * Semantics:
 *  - RelationTag is the identity of the relation (e.g. SBF.Relation.Family.Father).
 *  - AllyTags / EnemyTags list other relation tags this relation is allied
 *    with / hostile to. Including the relation's own tag marks the relation
 *    itself as an ally / enemy relation (self-inclusion convention).
 *  - bInheritsHostility: when true, ally stance is inherited transitively
 *    ("ally of my father is my ally"). Hostility always propagates up the
 *    hierarchy chain to the nearest common ancestor.
 *  - NodeWidgetClass: optional UUserWidget spawned above the NPC (visualization).
 */
UCLASS(BlueprintType, hidecategories = (Object))
class SOCIALBEHAVIORFRAMEWORK_API USBF_SocialRelation : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Identity tag of this relation type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relation", meta = (Categories = "SBF.Relation"))
	FGameplayTag RelationTag;

	/** Relation tags this relation considers allied. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relation", meta = (Categories = "SBF.Relation"))
	FGameplayTagContainer AllyTags;

	/** Relation tags this relation considers hostile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relation", meta = (Categories = "SBF.Relation"))
	FGameplayTagContainer EnemyTags;

	/** When true, ally stance is inherited through this relation transitively. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relation")
	bool bInheritsHostility = true;

	/** Visual hierarchy depth hint (used by editor layout / debug widgets). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relation", meta = (ClampMin = 0, ClampMax = 64))
	int32 HierarchyDepth = 1;

	/** Optional widget class displayed above NPCs using this relation (see USBF_NPCComponent). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
	TSubclassOf<UUserWidget> NodeWidgetClass;

	/** Editor / debug line color for edges using this relation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
	FLinearColor DebugColor = FLinearColor::White;

	/**
	 * @return the base hostility of this relation itself (self-inclusion check
	 *         against AllyTags / EnemyTags).
	 */
	UFUNCTION(BlueprintPure, Category = "SBF")
	ESBF_Hostility GetBaseHostility() const;

	/**
	 * @param OtherRelation another relation tag.
	 * @return how this relation stands towards the given relation tag.
	 */
	UFUNCTION(BlueprintPure, Category = "SBF")
	ESBF_Hostility GetHostilityTowards(FGameplayTag OtherRelation) const;

	/** @return display text derived from the relation tag. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	FText GetRelationDisplayName() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
