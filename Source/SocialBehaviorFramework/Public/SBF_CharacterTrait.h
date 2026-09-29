// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBF_Types.h"
#include "SBF_CharacterTrait.generated.h"

/**
 * Character Trait data asset: a hierarchy of reactions an NPC character
 * ("Aggressive", "Cautious" ...) can express. Each reaction maps a trigger
 * GameplayTag to an optional override behavior subtree plus blackboard
 * writes. Reactions are selected by Priority (desc), then Weight (desc).
 */
UCLASS(BlueprintType, hidecategories = (Object))
class SOCIALBEHAVIORFRAMEWORK_API USBF_CharacterTrait : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Root identity tag of the trait (e.g. SBF.Trait.Aggressive). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trait", meta = (Categories = "SBF.Trait"))
	FGameplayTag TraitTag;

	/** Ordered reaction table. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trait")
	TArray<FSBF_TraitReaction> Reactions;

	/**
	 * Collects all reactions matching the given tag (hierarchical match:
	 * TriggerTag "Threat" matches "Threat.Life").
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void FindReactionsForTag(FGameplayTag Tag, TArray<FSBF_TraitReaction>& OutReactions) const;

	/**
	 * Picks the single best reaction for a tag: highest Priority, then
	 * highest Weight.
	 * @return true when a reaction exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool PickBestReaction(FGameplayTag Tag, FSBF_TraitReaction& OutReaction) const;

	/** @return true when at least one reaction matches the tag. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool HasReactionForTag(FGameplayTag Tag) const;

	/** @return all trigger tags referenced by the reactions (deduplicated). */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void CollectTriggerTags(TArray<FGameplayTag>& OutTags) const;

	/**
	 * Structural validation used by the editor window.
	 * @param OutErrors appended localized error descriptions.
	 * @return true when the trait is usable.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool Validate(TArray<FText>& OutErrors) const;
};
