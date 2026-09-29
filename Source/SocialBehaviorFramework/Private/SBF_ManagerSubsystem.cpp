// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_ManagerSubsystem.h"

#include "Engine/World.h"
#include "SBF_BehaviorMap.h"
#include "SBF_CharacterTrait.h"
#include "SBF_CharacterTraitComponent.h"
#include "SBF_DeveloperSettings.h"
#include "SBF_Log.h"
#include "SBF_NPCComponent.h"
#include "SBF_SocialGraph.h"

void USBF_ManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadDefaultAssets();
}

void USBF_ManagerSubsystem::Deinitialize()
{
	RegisteredNPCs.Empty();
	Super::Deinitialize();
}

bool USBF_ManagerSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::EditorPreview;
}

void USBF_ManagerSubsystem::RegisterNPC(AActor* NPC, USBF_NPCComponent* Component)
{
	if (!IsValid(NPC) || !IsValid(Component))
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF: RegisterNPC rejected invalid arguments."));
		return;
	}

	const FGuid Guid = NPC->GetActorGuid();
	if (TWeakObjectPtr<USBF_NPCComponent>* Existing = RegisteredNPCs.Find(Guid))
	{
		if (Existing->Get() == Component)
		{
			return;
		}
		UE_LOG(LogSBF, Warning, TEXT("SBF: actor '%s' re-registered with a different component."), *GetNameSafe(NPC));
	}
	RegisteredNPCs.Add(Guid, Component);

	UE_LOG(LogSBF, Verbose, TEXT("SBF: registered NPC '%s' (%d total)."), *GetNameSafe(NPC), RegisteredNPCs.Num());
}

void USBF_ManagerSubsystem::UnregisterNPC(AActor* NPC)
{
	if (!IsValid(NPC))
	{
		return;
	}
	if (RegisteredNPCs.Remove(NPC->GetActorGuid()) > 0)
	{
		UE_LOG(LogSBF, Verbose, TEXT("SBF: unregistered NPC '%s' (%d total)."), *GetNameSafe(NPC), RegisteredNPCs.Num());
	}
}

void USBF_ManagerSubsystem::NotifyTag(AActor* NPC, FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return;
	}
	if (USBF_NPCComponent* Component = GetNPCComponent(NPC))
	{
		if (USBF_CharacterTraitComponent* TraitComponent = Component->GetTraitComponent())
		{
			TraitComponent->NotifyTagAdded(Tag);
			return;
		}
	}
	UE_LOG(LogSBF, Verbose, TEXT("SBF: NotifyTag '%s' on '%s' ignored (no trait component)."), *Tag.ToString(), *GetNameSafe(NPC));
}

void USBF_ManagerSubsystem::BroadcastTag(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return;
	}
	for (const TPair<FGuid, TWeakObjectPtr<USBF_NPCComponent>>& Pair : RegisteredNPCs)
	{
		if (Pair.Value.IsValid())
		{
			if (USBF_CharacterTraitComponent* TraitComponent = Pair.Value->GetTraitComponent())
			{
				TraitComponent->NotifyTagAdded(Tag);
			}
		}
	}
}

void USBF_ManagerSubsystem::BroadcastTimeUpdate()
{
	for (const TPair<FGuid, TWeakObjectPtr<USBF_NPCComponent>>& Pair : RegisteredNPCs)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->ReevaluateSchedule(/*bForce=*/false);
		}
	}
}

ESBF_Hostility USBF_ManagerSubsystem::QueryRelation(const AActor* A, const AActor* B) const
{
	if (!IsValid(A) || !IsValid(B))
	{
		return ESBF_Hostility::Unknown;
	}
	if (A == B)
	{
		return ESBF_Hostility::Neutral;
	}

	const USBF_NPCComponent* ComponentA = GetNPCComponent(A);
	const USBF_NPCComponent* ComponentB = GetNPCComponent(B);

	const USBF_SocialGraph* GraphA = ComponentA ? ComponentA->GetSocialGraph() : nullptr;
	const USBF_SocialGraph* GraphB = ComponentB ? ComponentB->GetSocialGraph() : nullptr;

	// Actors must share a graph (or at least one of them must be in one).
	const USBF_SocialGraph* Graph = GraphA ? GraphA : (GraphB ? GraphB : ActiveSocialGraph);
	if (!Graph)
	{
		return ESBF_Hostility::Unknown;
	}
	if (GraphA && GraphB && GraphA != GraphB)
	{
		UE_LOG(LogSBF, Verbose, TEXT("SBF: QueryRelation between actors from different graphs."));
		return ESBF_Hostility::Unknown;
	}

	return Graph->ResolveHostilityForActors(A->GetActorGuid(), B->GetActorGuid());
}

const USBF_BehaviorMap* USBF_ManagerSubsystem::GetBehaviorMapFor(const AActor* NPC) const
{
	if (const USBF_NPCComponent* Component = GetNPCComponent(NPC))
	{
		if (const USBF_BehaviorMap* Map = Component->GetBehaviorMap())
		{
			return Map;
		}
	}
	return DefaultBehaviorMap;
}

const USBF_CharacterTrait* USBF_ManagerSubsystem::GetTraitFor(const AActor* NPC) const
{
	if (const USBF_NPCComponent* Component = GetNPCComponent(NPC))
	{
		if (const USBF_CharacterTrait* Trait = Component->GetCharacterTrait())
		{
			return Trait;
		}
	}
	return DefaultCharacterTrait;
}

USBF_NPCComponent* USBF_ManagerSubsystem::GetNPCComponent(const AActor* NPC) const
{
	if (!IsValid(NPC))
	{
		return nullptr;
	}
	const TWeakObjectPtr<USBF_NPCComponent>* Found = RegisteredNPCs.Find(NPC->GetActorGuid());
	return (Found && Found->IsValid()) ? Found->Get() : nullptr;
}

void USBF_ManagerSubsystem::SetActiveSocialGraph(USBF_SocialGraph* Graph)
{
	ActiveSocialGraph = Graph;
}

void USBF_ManagerSubsystem::ReloadDefaultAssets()
{
	const USBF_DeveloperSettings& Settings = USBF_DeveloperSettings::Get();

	ActiveSocialGraph = Settings.DefaultSocialGraph.LoadSynchronous();
	DefaultBehaviorMap = Settings.DefaultBehaviorMap.LoadSynchronous();
	DefaultCharacterTrait = Settings.DefaultCharacterTrait.LoadSynchronous();
}
