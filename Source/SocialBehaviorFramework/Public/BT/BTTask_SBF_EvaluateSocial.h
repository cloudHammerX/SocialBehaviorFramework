// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BTTask_SBF_EvaluateSocial.generated.h"

/**
 * Behavior Tree task: evaluates the social stance of the AI pawn towards a
 * target actor read from the blackboard and writes the results:
 *   IsHostileKey  - true when the target is an enemy,
 *   IsAllyKey     - true when the target is an ally,
 *   RelationTagKey- name of the direct relation tag (e.g. "SBF.Relation.Enemy").
 *
 * The query goes through USBF_ManagerSubsystem::QueryRelation (cached).
 */
UCLASS(Category = "SBF")
class SOCIALBEHAVIORFRAMEWORK_API UBTTask_SBF_EvaluateSocial : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SBF_EvaluateSocial();

	/** Blackboard key holding the target actor. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	/** Output key: true when the target is hostile. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsHostileKey;

	/** Output key: true when the target is an ally. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsAllyKey;

	/** Output key: direct relation tag name. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector RelationTagKey;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual FName GetNodeIconName() const override;
};
