// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_TagLibrary.h"

#include "SBF_Log.h"
#include "UObject/Package.h"

bool USBF_TagLibrary::ContainsTag(FGameplayTag Tag) const
{
	return TriggerTags.Contains(Tag);
}

bool USBF_TagLibrary::AddTag(FGameplayTag Tag)
{
	if (!Tag.IsValid() || TriggerTags.Contains(Tag))
	{
		UE_LOG(LogSBF, Verbose, TEXT("TagLibrary '%s': AddTag skipped (invalid or duplicate tag)."), *GetName());
		return false;
	}
	TriggerTags.Add(Tag);
	TriggerTags.Sort([](const FGameplayTag& Lhs, const FGameplayTag& Rhs) { return Lhs.ToString() < Rhs.ToString(); });
#if WITH_EDITOR
	MarkPackageDirty();
#endif
	return true;
}

bool USBF_TagLibrary::RemoveTag(FGameplayTag Tag)
{
	const bool bRemoved = TriggerTags.Remove(Tag) > 0;
	if (bRemoved)
	{
#if WITH_EDITOR
		MarkPackageDirty();
#endif
	}
	return bRemoved;
}

bool USBF_TagLibrary::RenameTag(FGameplayTag OldTag, FGameplayTag NewTag)
{
	if (OldTag == NewTag)
	{
		return false;
	}
	if (!NewTag.IsValid())
	{
		UE_LOG(LogSBF, Warning, TEXT("TagLibrary '%s': RenameTag failed, new tag is invalid."), *GetName());
		return false;
	}
	if (TriggerTags.Contains(NewTag))
	{
		UE_LOG(LogSBF, Warning, TEXT("TagLibrary '%s': RenameTag failed, '%s' already exists."), *GetName(), *NewTag.ToString());
		return false;
	}

	const int32 Index = TriggerTags.IndexOfByKey(OldTag);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	TriggerTags[Index] = NewTag;
#if WITH_EDITOR
	MarkPackageDirty();
#endif
	return true;
}

void USBF_TagLibrary::LoadDefaultTags()
{
	TriggerTags = GetDefaultTriggerTags();
#if WITH_EDITOR
	MarkPackageDirty();
#endif
}

bool USBF_TagLibrary::Validate(TArray<FText>& OutErrors) const
{
	bool bValid = true;
	if (TriggerTags.Num() == 0)
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_TagLibraryEmpty", "Tag Library: the tag set is empty."));
		bValid = false;
	}
	return bValid;
}

TArray<FGameplayTag> USBF_TagLibrary::GetDefaultTriggerTags()
{
	static const TArray<FName> DefaultTagNames =
	{
		FName(TEXT("Threat.Life")),
		FName(TEXT("Threat.Weapon")),
		FName(TEXT("Threat.Sound")),
		FName(TEXT("Work.Late")),
		FName(TEXT("Work.Overtime")),
		FName(TEXT("Offender.Detected")),
		FName(TEXT("Offender.Identified")),
		FName(TEXT("Ally.InDanger")),
		FName(TEXT("Ally.NeedsHelp"))
	};

	TArray<FGameplayTag> Tags;
	Tags.Reserve(DefaultTagNames.Num());
	for (const FName& Name : DefaultTagNames)
	{
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(Name, /*bErrorIfNotFound=*/false);
		if (Tag.IsValid())
		{
			Tags.Add(Tag);
		}
	}
	return Tags;
}
