// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BTTask_SBF_UpdateSchedule.generated.h"

/**
 * Behavior Tree task: forces an immediate schedule re-evaluation on the AI's
 * USBF_NPCComponent and writes the result into the blackboard:
 *   TargetLocationKey - resolved target location (Home/Work/Leisure),
 *   ActivityKey       - activity name ("Home", "Work", "Leisure").
 */
UCLASS(Category = "SBF")
class SOCIALBEHAVIORFRAMEWORK_API UBTTask_SBF_UpdateSchedule : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SBF_UpdateSchedule();

	/** Output key: schedule target location. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	/** Output key: activity name. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ActivityKey;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual FName GetNodeIconName() const override;
};
