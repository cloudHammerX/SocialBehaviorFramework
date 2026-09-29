// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SBF_Types.h"
#include "SBF_NPCComponent.generated.h"

class USBF_SocialGraph;
class USBF_BehaviorMap;
class USBF_CharacterTrait;
class USBF_CharacterTraitComponent;
class UBlackboardComponent;
class UWidgetComponent;
class USBF_TimeSubsystem;

/**
 * Per-NPC component tying the three SBF subsystems together.
 *
 *  - subscribes to USBF_TimeSubsystem::OnMinuteTick and re-evaluates the
 *    schedule of its USBF_BehaviorMap (event driven, no per-NPC timers);
 *  - writes BB_TargetLocation / BB_CurrentActivity into the AI blackboard;
 *  - optionally auto-moves the owning pawn to the current target;
 *  - registers itself with USBF_ManagerSubsystem for relation queries;
 *  - optionally spawns the relation widget (UUserWidget from the relation
 *    asset's NodeWidgetClass) above the pawn.
 */
UCLASS(ClassGroup = (SBF), meta = (BlueprintSpawnableComponent, DisplayName = "SBF NPC Component"))
class SOCIALBEHAVIORFRAMEWORK_API USBF_NPCComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USBF_NPCComponent();

	/** Social graph driving this NPC's relationships. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Assets")
	TObjectPtr<USBF_SocialGraph> SocialGraph = nullptr;

	/** Behavior map driving this NPC's daily schedule. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Assets")
	TObjectPtr<USBF_BehaviorMap> BehaviorMap = nullptr;

	/** Character trait driving this NPC's tag reactions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Assets")
	TObjectPtr<USBF_CharacterTrait> CharacterTrait = nullptr;

	/** When true the owning pawn auto-moves to the resolved schedule target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Movement")
	bool bAutoMove = true;

	/** Acceptance radius used by the auto-move request. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Movement", meta = (ClampMin = "0.0"))
	float MoveAcceptanceRadius = 50.0f;

	/** When true and the relation asset defines a NodeWidgetClass, spawn it above the pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Visualization")
	bool bSpawnRelationWidget = true;

	/** Blackboard key receiving the schedule target location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Blackboard")
	FName TargetLocationKey = FName(TEXT("BB_TargetLocation"));

	/** Blackboard key receiving the current activity name (Home/Work/Leisure). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF|Blackboard")
	FName ActivityKey = FName(TEXT("BB_CurrentActivity"));

	/** Fired when the resolved schedule target changes. */
	UPROPERTY(BlueprintAssignable, Category = "SBF")
	FOnSBF_ScheduleChanged OnScheduleChanged;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Re-evaluates the schedule against the current game time and updates the
	 * blackboard / movement. No-op when the result matches the cached one
	 * (unless bForce is set).
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void ReevaluateSchedule(bool bForce = false);

	/** @return the stance towards another actor via the manager subsystem. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	ESBF_Hostility QueryRelationTowards(const AActor* Other) const;

	/** @return the currently resolved activity. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	ESBF_ActivityType GetCurrentActivity() const { return CachedActivity; }

	/** @return the currently resolved target location. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	FVector GetTargetLocation() const { return CachedTargetLocation; }

	/** @return the owned (or auto-created) character trait component. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	USBF_CharacterTraitComponent* GetTraitComponent() const;

	/** @return the assigned social graph (may be nullptr). */
	const USBF_SocialGraph* GetSocialGraph() const { return SocialGraph; }

	/** @return the assigned behavior map (may be nullptr). */
	const USBF_BehaviorMap* GetBehaviorMap() const { return BehaviorMap; }

	/** @return the assigned character trait (may be nullptr). */
	const USBF_CharacterTrait* GetCharacterTrait() const { return CharacterTrait; }

	/** @return the relation tag of this NPC's node in its social graph. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	FGameplayTag GetOwnRelationTag() const;

private:
	/** Time subsystem delegate handler for minute ticks. */
	void HandleMinuteTick(const FSBF_GameTime& GameTime);

	/** Resolves the blackboard component (via the owning AI controller) and caches it. */
	UBlackboardComponent* GetCachedBlackboard() const;

	/** Writes the cached schedule state into the blackboard. */
	void WriteBlackboard();

	/** Spawns the relation widget above the owner when configured. */
	void SpawnRelationWidget();

	/** Resolves the relation asset of this NPC's node. */
	const class USBF_SocialRelation* ResolveOwnRelation() const;

	ESBF_ActivityType CachedActivity = ESBF_ActivityType::None;
	FVector CachedTargetLocation = FVector::ZeroVector;

	mutable TWeakObjectPtr<UBlackboardComponent> CachedBlackboard;
	mutable TWeakObjectPtr<USBF_CharacterTraitComponent> CachedTraitComponent;
	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> RelationWidgetComponent = nullptr;

	FDelegateHandle MinuteTickHandle;
};
