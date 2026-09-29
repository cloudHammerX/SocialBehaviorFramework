// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_SocialRelation.h"

#include "SBF_Log.h"

ESBF_Hostility USBF_SocialRelation::GetBaseHostility() const
{
	if (!RelationTag.IsValid())
	{
		return ESBF_Hostility::Neutral;
	}
	if (EnemyTags.HasTagExact(RelationTag))
	{
		return ESBF_Hostility::Enemy;
	}
	if (AllyTags.HasTagExact(RelationTag))
	{
		return ESBF_Hostility::Ally;
	}
	return ESBF_Hostility::Neutral;
}

ESBF_Hostility USBF_SocialRelation::GetHostilityTowards(FGameplayTag OtherRelation) const
{
	if (!OtherRelation.IsValid())
	{
		return ESBF_Hostility::Neutral;
	}
	if (EnemyTags.HasTagExact(OtherRelation))
	{
		return ESBF_Hostility::Enemy;
	}
	if (AllyTags.HasTagExact(OtherRelation))
	{
		return ESBF_Hostility::Ally;
	}
	return ESBF_Hostility::Neutral;
}

FText USBF_SocialRelation::GetRelationDisplayName() const
{
	if (!RelationTag.IsValid())
	{
		return NSLOCTEXT("SBF", "UnnamedRelation", "Unnamed Relation");
	}
	return FText::FromName(RelationTag.GetTagName());
}

#if WITH_EDITOR
void USBF_SocialRelation::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (const FProperty* Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(USBF_SocialRelation, RelationTag))
		{
			UE_LOG(LogSBF, Verbose, TEXT("Social relation tag changed: %s"), *RelationTag.ToString());
		}
	}
}
#endif
