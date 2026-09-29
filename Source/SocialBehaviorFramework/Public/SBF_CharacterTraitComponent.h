// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SBF_Types.h"
#include "SBF_CharacterTraitComponent.generated.h"

class USBF_CharacterTrait;
class USBF_TagLibrary;

/**
 * Per-NPC component hosting the Character Trait subsystem.
 *
 * Tags arrive through USBF_ManagerSubsystem::NotifyTag / BroadcastTag (no
 * polling). On every tag the component matches its trait reactions
 * (hierarchical TriggerTag match), keeps the pending reaction list ordered by
 * Priority (desc) then Weight (desc) and exposes the best pending reaction for
 * BTTask_SBF_ApplyTrait / BTTask_UpdateSchedule consumers.
 */
UCLASS(ClassGroup = (SBF), meta = (BlueprintSpawnableComponent, DisplayName = "SBF Character Trait Component"))
class SOCIALBEHAVIORFRAMEWORK_API USBF_CharacterTraitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USBF_CharacterTraitComponent();

	/** Trait asset driving the reactions (may be null until assigned by NPC component). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	TObjectPtr<USBF_CharacterTrait> CharacterTrait = nullptr;

	/** Optional tag library used for editor tooling / validation only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	TObjectPtr<USBF_TagLibrary> TagLibrary = nullptr;

	/** Tags the NPC currently reacts to (debug / UI). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "SBF")
	FGameplayTagContainer ActiveReactionTags;

	/** Fired when a trait reaction triggers on this NPC. */
	UPROPERTY(BlueprintAssignable, Category = "SBF")
	FOnSBF_TraitTriggered OnTraitTriggered;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Notifies the component that the owning NPC gained a tag. Matches the
	 * trait reactions and stores the best pending reaction.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void NotifyTagAdded(FGameplayTag Tag);

	/** Clears all pending reactions. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void ClearReactions();

	/** @return true when a reaction is pending for the given tag (hierarchical). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool HasReactionFor(FGameplayTag Tag) const;

	/**
	 * @return the best pending reaction (Priority desc, Weight desc), or false
	 *         when the reaction queue is empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool GetBestPendingReaction(FSBF_TraitReaction& OutReaction) const;

	/**
	 * Marks a reaction (by exact trigger tag) as consumed.
	 * @return true when a pending reaction was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool ConsumeReaction(FGameplayTag TriggerTag);

	/** @return the number of currently pending reactions. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	int32 GetPendingReactionCount() const { return PendingReactions.Num(); }

private:
	/** Pending reactions ordered by Priority (desc) then Weight (desc). */
	TArray<FSBF_TraitReaction> PendingReactions;

	void EvaluateReactions(FGameplayTag Tag);
	void InsertSorted(const FSBF_TraitReaction& Reaction);
};
