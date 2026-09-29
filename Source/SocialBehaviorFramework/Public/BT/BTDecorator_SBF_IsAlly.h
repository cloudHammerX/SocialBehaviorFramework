// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BTDecorator_SBF_IsAlly.generated.h"

/**
 * Behavior Tree decorator: true while the AI's stance towards the actor
 * stored in TargetActorKey is allied (see USBF_ManagerSubsystem::QueryRelation).
 */
UCLASS(Category = "SBF")
class SOCIALBEHAVIORFRAMEWORK_API UBTDecorator_SBF_IsAlly : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_SBF_IsAlly();

	/** Blackboard key holding the target actor. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	virtual FName GetNodeIconName() const override;

private:
	bool Evaluate(const UBehaviorTreeComponent& OwnerComp) const;
};
