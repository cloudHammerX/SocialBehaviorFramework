// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"
#include "BTDecorator_SBF_TagReaction.generated.h"

/**
 * Behavior Tree decorator: true while a reaction for the given TriggerTag is
 * pending on the AI pawn's USBF_CharacterTraitComponent (hierarchical match:
 * TriggerTag "Threat" also matches "Threat.Life"). Set bInverse to gate the
 * branch on the ABSENCE of the reaction.
 */
UCLASS(Category = "SBF")
class SOCIALBEHAVIORFRAMEWORK_API UBTDecorator_SBF_TagReaction : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_SBF_TagReaction();

	/** Reaction trigger tag to watch. */
	UPROPERTY(EditAnywhere, Category = "SBF", meta = (Categories = "SBF"))
	FGameplayTag TriggerTag;

	/** When true, the condition is inverted. */
	UPROPERTY(EditAnywhere, Category = "SBF")
	bool bInverse = false;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	virtual FName GetNodeIconName() const override;
};
