// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BTTask_SBF_ApplyTrait.generated.h"

/**
 * Behavior Tree task: applies the best pending trait reaction of the AI pawn.
 *
 * The reaction is selected by the trait component (Priority desc, then Weight
 * desc). The task:
 *   - writes the reaction's BlackboardKeysToSet (bool keys -> true, name keys
 *     -> the trigger tag name);
 *   - writes the trigger tag name into ReactionTagKey;
 *   - when the reaction defines an OverrideSubtree, pushes it on the behavior
 *     tree execution stack and stays InProgress until the subtree finishes
 *     (same mechanism the engine's UBTTask_RunBehavior uses).
 */
UCLASS(Category = "SBF")
class SOCIALBEHAVIORFRAMEWORK_API UBTTask_SBF_ApplyTrait : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SBF_ApplyTrait();

	/** Output key receiving the applied reaction trigger tag name. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ReactionTagKey;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual void OnTaskAborted(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	virtual FName GetNodeIconName() const override;

private:
	struct FBTTask_SBF_ApplyTraitMemory
	{
		bool bSubtreeRunning = false;
		int32 PushedInstanceIdx = INDEX_NONE;
	};

	void WriteReactionKeys(UBehaviorTreeComponent& OwnerComp, const FSBF_TraitReaction& Reaction) const;
};
