// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_CharacterTrait.h"

#include "SBF_Log.h"

void USBF_CharacterTrait::FindReactionsForTag(FGameplayTag Tag, TArray<FSBF_TraitReaction>& OutReactions) const
{
	OutReactions.Empty();
	if (!Tag.IsValid())
	{
		return;
	}
	for (const FSBF_TraitReaction& Reaction : Reactions)
	{
		if (Tag.MatchesTag(Reaction.TriggerTag))
		{
			OutReactions.Add(Reaction);
		}
	}
}

bool USBF_CharacterTrait::PickBestReaction(FGameplayTag Tag, FSBF_TraitReaction& OutReaction) const
{
	const FSBF_TraitReaction* Best = nullptr;
	for (const FSBF_TraitReaction& Reaction : Reactions)
	{
		if (!Tag.MatchesTag(Reaction.TriggerTag))
		{
			continue;
		}
		if (!Best || Reaction.Priority > Best->Priority || (Reaction.Priority == Best->Priority && Reaction.Weight > Best->Weight))
		{
			Best = &Reaction;
		}
	}

	if (!Best)
	{
		return false;
	}
	OutReaction = *Best;
	return true;
}

bool USBF_CharacterTrait::HasReactionForTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid())
	{
		return false;
	}
	for (const FSBF_TraitReaction& Reaction : Reactions)
	{
		if (Tag.MatchesTag(Reaction.TriggerTag))
		{
			return true;
		}
	}
	return false;
}

void USBF_CharacterTrait::CollectTriggerTags(TArray<FGameplayTag>& OutTags) const
{
	OutTags.Empty();
	for (const FSBF_TraitReaction& Reaction : Reactions)
	{
		OutTags.AddUnique(Reaction.TriggerTag);
	}
}

bool USBF_CharacterTrait::Validate(TArray<FText>& OutErrors) const
{
	bool bValid = true;

	if (!TraitTag.IsValid())
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_TraitNoRoot", "Character Trait: TraitTag is not set."));
		bValid = false;
	}

	for (int32 Index = 0; Index < Reactions.Num(); ++Index)
	{
		const FSBF_TraitReaction& Reaction = Reactions[Index];
		if (!Reaction.TriggerTag.IsValid())
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Character Trait: reaction #%d has an empty TriggerTag."), Index)));
			bValid = false;
		}
		if (Reaction.OverrideSubtree == nullptr && Reaction.BlackboardKeysToSet.Num() == 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Character Trait: reaction #%d has neither a subtree nor blackboard keys."), Index)));
			bValid = false;
		}
	}

	if (Reactions.Num() == 0)
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_TraitNoReactions", "Character Trait: no reactions defined."));
		bValid = false;
	}

	return bValid;
}
