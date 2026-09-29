// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_AssetFactories.h"

#include "SBF_BehaviorMap.h"
#include "SBF_CharacterTrait.h"
#include "SBF_Log.h"
#include "SBF_SocialGraph.h"
#include "SBF_SocialRelation.h"
#include "SBF_TagLibrary.h"

// =============================================================================
// USBF_SocialGraphFactory
// =============================================================================

USBF_SocialGraphFactory::USBF_SocialGraphFactory()
{
	SupportedClass = USBF_SocialGraph::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USBF_SocialGraphFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USBF_SocialGraph* Graph = NewObject<USBF_SocialGraph>(InParent, InClass, InName, Flags | RF_Transactional);

	// Default relation set (sub-assets saved with the graph).
	auto CreateRelation = [Graph](const FName& TagName, const FLinearColor& Color) -> USBF_SocialRelation*
	{
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, /*bErrorIfNotFound=*/false);
		if (!Tag.IsValid())
		{
			return nullptr;
		}

		USBF_SocialRelation* Relation = NewObject<USBF_SocialRelation>(Graph, NAME_None, RF_Public | RF_Transactional);
		Relation->RelationTag = Tag;
		Relation->DebugColor = Color;
		return Relation;
	};

	// Friend: allied relation (self-inclusion in AllyTags).
	if (USBF_SocialRelation* Friend = CreateRelation(FName(TEXT("SBF.Relation.Friend")), FLinearColor(0.2f, 0.8f, 0.3f)))
	{
		Friend->AllyTags.AddTag(Friend->RelationTag);
		Graph->RelationAssets.Add(Friend);
	}

	// Enemy: hostile relation (self-inclusion in EnemyTags).
	if (USBF_SocialRelation* Enemy = CreateRelation(FName(TEXT("SBF.Relation.Enemy")), FLinearColor(0.9f, 0.2f, 0.2f)))
	{
		Enemy->EnemyTags.AddTag(Enemy->RelationTag);
		Graph->RelationAssets.Add(Enemy);
	}

	// Family.Father: allied family relation.
	if (USBF_SocialRelation* Father = CreateRelation(FName(TEXT("SBF.Relation.Family.Father")), FLinearColor(0.3f, 0.5f, 0.9f)))
	{
		Father->AllyTags.AddTag(Father->RelationTag);
		Graph->RelationAssets.Add(Father);
	}

	UE_LOG(LogSBF, Log, TEXT("Created Social Graph '%s' with %d default relations."), *InName.ToString(), Graph->RelationAssets.Num());
	return Graph;
}

// =============================================================================
// USBF_BehaviorMapFactory
// =============================================================================

USBF_BehaviorMapFactory::USBF_BehaviorMapFactory()
{
	SupportedClass = USBF_BehaviorMap::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USBF_BehaviorMapFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USBF_BehaviorMap* Map = NewObject<USBF_BehaviorMap>(InParent, InClass, InName, Flags | RF_Transactional);

	Map->WorkHours.StartHour = 9;
	Map->WorkHours.StartMinute = 0;
	Map->WorkHours.EndHour = 18;
	Map->WorkHours.EndMinute = 0;

	Map->DefaultLeisureHours.StartHour = 18;
	Map->DefaultLeisureHours.StartMinute = 0;
	Map->DefaultLeisureHours.EndHour = 22;
	Map->DefaultLeisureHours.EndMinute = 0;

	UE_LOG(LogSBF, Log, TEXT("Created Behavior Map '%s' with a default 9-18 work day."), *InName.ToString());
	return Map;
}

// =============================================================================
// USBF_CharacterTraitFactory
// =============================================================================

USBF_CharacterTraitFactory::USBF_CharacterTraitFactory()
{
	SupportedClass = USBF_CharacterTrait::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USBF_CharacterTraitFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USBF_CharacterTrait* Trait = NewObject<USBF_CharacterTrait>(InParent, InClass, InName, Flags | RF_Transactional);

	Trait->TraitTag = FGameplayTag::RequestGameplayTag(FName(TEXT("SBF.Trait.Aggressive")), /*bErrorIfNotFound=*/false);

	auto AddReaction = [Trait](const FName& TriggerName, int32 Priority, float Weight, const TArray<FName>& Keys)
	{
		const FGameplayTag Trigger = FGameplayTag::RequestGameplayTag(TriggerName, /*bErrorIfNotFound=*/false);
		if (!Trigger.IsValid())
		{
			return;
		}
		FSBF_TraitReaction Reaction;
		Reaction.TriggerTag = Trigger;
		Reaction.Priority = Priority;
		Reaction.Weight = Weight;
		Reaction.BlackboardKeysToSet = Keys;
		Trait->Reactions.Add(Reaction);
	};

	AddReaction(FName(TEXT("Threat.Life")), 100, 1.0f, { FName(TEXT("bThreatened")), FName(TEXT("bCombatMode")) });
	AddReaction(FName(TEXT("Offender.Detected")), 80, 1.0f, { FName(TEXT("bInvestigating")) });
	AddReaction(FName(TEXT("Ally.InDanger")), 60, 1.0f, { FName(TEXT("bHelpAlly")) });

	UE_LOG(LogSBF, Log, TEXT("Created Character Trait '%s' with %d default reactions."), *InName.ToString(), Trait->Reactions.Num());
	return Trait;
}

// =============================================================================
// USBF_TagLibraryFactory
// =============================================================================

USBF_TagLibraryFactory::USBF_TagLibraryFactory()
{
	SupportedClass = USBF_TagLibrary::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USBF_TagLibraryFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USBF_TagLibrary* Library = NewObject<USBF_TagLibrary>(InParent, InClass, InName, Flags | RF_Transactional);
	Library->TriggerTags = USBF_TagLibrary::GetDefaultTriggerTags();

	UE_LOG(LogSBF, Log, TEXT("Created Tag Library '%s' with %d default tags."), *InName.ToString(), Library->TriggerTags.Num());
	return Library;
}

// =============================================================================
// USBF_SocialRelationFactory
// =============================================================================

USBF_SocialRelationFactory::USBF_SocialRelationFactory()
{
	SupportedClass = USBF_SocialRelation::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USBF_SocialRelationFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<USBF_SocialRelation>(InParent, InClass, InName, Flags | RF_Transactional);
}
