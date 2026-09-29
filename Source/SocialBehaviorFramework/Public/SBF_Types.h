// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBF_Types.generated.h"

class UBehaviorTree;
class USBF_SocialRelation;

/**
 * Stance between two actors as resolved by the Social Graph subsystem.
 * Unknown  - no resolvable path / missing graph data.
 * Neutral  - actors know each other but share no allegiance.
 * Ally     - allied through an inherited relation.
 * Enemy    - hostile through a relation on the path (hostility propagates).
 */
UENUM(BlueprintType)
enum class ESBF_Hostility : uint8
{
	Unknown		UMETA(DisplayName = "Unknown"),
	Neutral		UMETA(DisplayName = "Neutral"),
	Ally		UMETA(DisplayName = "Ally"),
	Enemy		UMETA(DisplayName = "Enemy")
};

/**
 * Current activity of an NPC, resolved from its Behavior Map.
 */
UENUM(BlueprintType)
enum class ESBF_ActivityType : uint8
{
	None		UMETA(DisplayName = "None"),
	Home		UMETA(DisplayName = "Home"),
	Work		UMETA(DisplayName = "Work"),
	Leisure		UMETA(DisplayName = "Leisure")
};

/**
 * Immutable snapshot of the game world time managed by USBF_TimeSubsystem.
 * Day is 1-based (Day 1 == Monday). Weekday is an FName ("Monday" .. "Sunday").
 */
USTRUCT(BlueprintType)
struct SOCIALBEHAVIORFRAMEWORK_API FSBF_GameTime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	int32 Hour = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	int32 Minute = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	int32 Day = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FName Weekday = FName(TEXT("Monday"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	bool bIsWeekend = false;

	/** @return minutes elapsed since midnight. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	int32 TotalMinuteOfDay() const { return Hour * 60 + Minute; }

	/** Builds a time snapshot from an absolute minute counter. @param TotalMinutes minutes since day 0, 00:00. */
	static FSBF_GameTime FromTotalMinutes(int64 TotalMinutes, const TArray<FName>& WeekendDays);

	/** @return 0-based weekday index (0 == Monday). */
	int32 GetWeekdayIndex() const { return (FMath::Max(0, Day - 1)) % 7; }
};

/**
 * Day time range expressed in hours/minutes. Supports wrap-around ranges
 * (e.g. 22:00 - 06:00 is treated as "overnight").
 */
USTRUCT(BlueprintType)
struct SOCIALBEHAVIORFRAMEWORK_API FTimeRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (ClampMin = 0, ClampMax = 23))
	int32 StartHour = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (ClampMin = 0, ClampMax = 59))
	int32 StartMinute = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (ClampMin = 0, ClampMax = 23))
	int32 EndHour = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (ClampMin = 0, ClampMax = 59))
	int32 EndMinute = 0;

	UFUNCTION(BlueprintPure, Category = "SBF")
	int32 StartMinuteOfDay() const { return StartHour * 60 + StartMinute; }

	UFUNCTION(BlueprintPure, Category = "SBF")
	int32 EndMinuteOfDay() const { return EndHour * 60 + EndMinute; }

	/** @return true when MinuteOfDay falls into the range (wrap-around aware). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool Contains(int32 MinuteOfDay) const
	{
		const int32 Start = StartMinuteOfDay();
		const int32 End = EndMinuteOfDay();
		if (Start == End) { return false; }
		return Start < End ? (MinuteOfDay >= Start && MinuteOfDay < End)
						   : (MinuteOfDay >= Start || MinuteOfDay < End);
	}

	/** @return true when the range is well formed (start != end). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool IsValid() const { return StartMinuteOfDay() != EndMinuteOfDay(); }
};

/**
 * A single reaction entry of a Character Trait.
 * When TriggerTag fires on an NPC, the reaction with the highest Priority
 * (then Weight) wins and may override the running behavior subtree.
 */
USTRUCT(BlueprintType)
struct SOCIALBEHAVIORFRAMEWORK_API FSBF_TraitReaction
{
	GENERATED_BODY()

	/** Trigger tag; hierarchical (reaction with TriggerTag "Threat" fires for "Threat.Life"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (Categories = "SBF"))
	FGameplayTag TriggerTag;

	/** Optional behavior subtree injected when this reaction is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	TObjectPtr<UBehaviorTree> OverrideSubtree = nullptr;

	/** Selection weight, used as a tie breaker after Priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (ClampMin = 0.0f))
	float Weight = 1.0f;

	/** Selection priority; higher wins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	int32 Priority = 0;

	/** Blackboard keys written by BTTask_SBF_ApplyTrait (bool keys -> true, name keys -> trigger tag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	TArray<FName> BlackboardKeysToSet;
};

/**
 * A node of the social graph. ActorGuid links the node to a world actor
 * registered through USBF_ManagerSubsystem / USBF_NPCComponent.
 * ParentGuid + RelationTag describe the hierarchy edge to the parent node.
 */
USTRUCT(BlueprintType)
struct SOCIALBEHAVIORFRAMEWORK_API FSBF_SocialNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FGuid NodeGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FGuid ActorGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FName DisplayName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FGuid ParentGuid;

	/** Relation of this node towards its parent (identity of a USBF_SocialRelation). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (Categories = "SBF.Relation"))
	FGameplayTag RelationTag;
};

/**
 * An explicit, non-hierarchical edge between two nodes (e.g. "friend of",
 * "vendetta with"). Optional relation asset reference is used first when
 * resolving hostility.
 */
USTRUCT(BlueprintType)
struct SOCIALBEHAVIORFRAMEWORK_API FSBF_SocialEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FGuid EdgeGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FGuid NodeA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	FGuid NodeB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF", meta = (Categories = "SBF.Relation"))
	FGameplayTag RelationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	TObjectPtr<USBF_SocialRelation> Relation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SBF")
	bool bBidirectional = true;
};

/** Broadcast every game minute with the new time snapshot. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameTimeChanged, const FSBF_GameTime& /*NewTime*/);

/** Broadcast when an NPC changes its schedule target. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSBF_ScheduleChanged, ESBF_ActivityType, Activity, FVector, TargetLocation);

/** Broadcast when a trait reaction fires on an NPC. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSBF_TraitTriggered, FGameplayTag, TriggerTag, int32, Priority);
