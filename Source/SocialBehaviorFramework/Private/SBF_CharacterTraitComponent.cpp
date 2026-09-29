// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_CharacterTraitComponent.h"

#include "SBF_CharacterTrait.h"
#include "SBF_Log.h"
#include "SBF_TagLibrary.h"

USBF_CharacterTraitComponent::USBF_CharacterTraitComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USBF_CharacterTraitComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PendingReactions.Empty();
	ActiveReactionTags.Reset();
	Super::EndPlay(EndPlayReason);
}

void USBF_CharacterTraitComponent::NotifyTagAdded(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return;
	}

	if (!CharacterTrait)
	{
		UE_LOG(LogSBF, Verbose, TEXT("SBF: NPC '%s' has no CharacterTrait, tag '%s' ignored."), *GetNameSafe(GetOwner()), *Tag.ToString());
		return;
	}

	EvaluateReactions(Tag);
}

void USBF_CharacterTraitComponent::EvaluateReactions(FGameplayTag Tag)
{
	TArray<FSBF_TraitReaction> Matches;
	CharacterTrait->FindReactionsForTag(Tag, Matches);

	for (const FSBF_TraitReaction& Reaction : Matches)
	{
		// One pending entry per trigger tag.
		const bool bAlreadyPending = PendingReactions.ContainsByPredicate(
			[&Reaction](const FSBF_TraitReaction& Pending) { return Pending.TriggerTag == Reaction.TriggerTag; });
		if (bAlreadyPending)
		{
			continue;
		}

		InsertSorted(Reaction);
		ActiveReactionTags.AddTag(Tag);

		OnTraitTriggered.Broadcast(Reaction.TriggerTag, Reaction.Priority);
		UE_LOG(LogSBF, Log, TEXT("SBF: NPC '%s' triggered reaction '%s' (priority %d, weight %.2f)."),
			*GetNameSafe(GetOwner()), *Reaction.TriggerTag.ToString(), Reaction.Priority, Reaction.Weight);
	}
}

void USBF_CharacterTraitComponent::InsertSorted(const FSBF_TraitReaction& Reaction)
{
	int32 InsertIndex = 0;
	while (InsertIndex < PendingReactions.Num())
	{
		const FSBF_TraitReaction& Existing = PendingReactions[InsertIndex];
		if (Reaction.Priority > Existing.Priority ||
			(Reaction.Priority == Existing.Priority && Reaction.Weight > Existing.Weight))
		{
			break;
		}
		++InsertIndex;
	}
	PendingReactions.Insert(Reaction, InsertIndex);
}

void USBF_CharacterTraitComponent::ClearReactions()
{
	PendingReactions.Empty();
	ActiveReactionTags.Reset();
}

bool USBF_CharacterTraitComponent::HasReactionFor(FGameplayTag Tag) const
{
	if (!Tag.IsValid())
	{
		return false;
	}
	return PendingReactions.ContainsByPredicate([Tag](const FSBF_TraitReaction& Pending)
	{
		return Tag.MatchesTag(Pending.TriggerTag);
	});
}

bool USBF_CharacterTraitComponent::GetBestPendingReaction(FSBF_TraitReaction& OutReaction) const
{
	if (PendingReactions.Num() == 0)
	{
		return false;
	}
	OutReaction = PendingReactions[0];
	return true;
}

bool USBF_CharacterTraitComponent::ConsumeReaction(FGameplayTag TriggerTag)
{
	const int32 Removed = PendingReactions.RemoveAll([TriggerTag](const FSBF_TraitReaction& Pending)
	{
		return Pending.TriggerTag == TriggerTag;
	});
	if (Removed > 0)
	{
		ActiveReactionTags.RemoveTag(TriggerTag);
		return true;
	}
	return false;
}
