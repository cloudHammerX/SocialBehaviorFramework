// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "SBF_Types.h"
#include "SBF_ManagerSubsystem.generated.h"

class USBF_NPCComponent;
class USBF_SocialGraph;
class USBF_BehaviorMap;
class USBF_CharacterTrait;

/**
 * Runtime facade of the Social Behavior Framework: the single entry point
 * NPCs and gameplay code talk to.
 *
 * Responsibilities:
 *  - NPC registry (actor guid -> component) for O(1) lookups;
 *  - relationship queries with the Social Graph (hostility cache lives in
 *    the graph asset, shared across all NPCs);
 *  - tag fan-out for the Character Trait subsystem;
 *  - schedule broadcast for the Behavior Map subsystem.
 */
UCLASS()
class SOCIALBEHAVIORFRAMEWORK_API USBF_ManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/**
	 * Registers an NPC (idempotent).
	 * @param NPC the world actor representing the NPC.
	 * @param Component the NPC's USBF_NPCComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void RegisterNPC(AActor* NPC, USBF_NPCComponent* Component);

	/** Unregisters an NPC (idempotent). */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void UnregisterNPC(AActor* NPC);

	/**
	 * Notifies the framework that an NPC gained a tag ("Threat.Life" ...).
	 * Routed to the NPC's trait component, which may fire a reaction.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void NotifyTag(AActor* NPC, FGameplayTag Tag);

	/** Fans a tag out to every registered NPC (world event, e.g. "Ally.InDanger"). */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void BroadcastTag(FGameplayTag Tag);

	/** Forces every registered NPC to re-evaluate its schedule (no per-NPC timers). */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void BroadcastTimeUpdate();

	/**
	 * Resolves the relationship between two actors through their social graphs.
	 * @return hostility stance (Unknown when either actor is not in a graph).
	 */
	UFUNCTION(BlueprintPure, Category = "SBF")
	ESBF_Hostility QueryRelation(const AActor* A, const AActor* B) const;

	/** @return the active (fallback) social graph asset. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	const USBF_SocialGraph* GetActiveSocialGraph() const { return ActiveSocialGraph; }

	/** @return the behavior map of an NPC (falls back to the project default). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	const USBF_BehaviorMap* GetBehaviorMapFor(const AActor* NPC) const;

	/** @return the character trait of an NPC (falls back to the project default). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	const USBF_CharacterTrait* GetTraitFor(const AActor* NPC) const;

	/** @return the registered NPC component of an actor (nullptr when unregistered). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	USBF_NPCComponent* GetNPCComponent(const AActor* NPC) const;

	/** Overrides the active (fallback) social graph at runtime. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void SetActiveSocialGraph(USBF_SocialGraph* Graph);

	/** Re-reads the fallback assets from USBF_DeveloperSettings. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void ReloadDefaultAssets();

private:
	UPROPERTY(Transient)
	TObjectPtr<USBF_SocialGraph> ActiveSocialGraph = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USBF_BehaviorMap> DefaultBehaviorMap = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USBF_CharacterTrait> DefaultCharacterTrait = nullptr;

	/** Actor guid -> NPC component registry. */
	TMap<FGuid, TWeakObjectPtr<USBF_NPCComponent>> RegisteredNPCs;
};
