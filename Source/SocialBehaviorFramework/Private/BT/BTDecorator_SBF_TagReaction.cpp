// Copyright Social Behavior Framework. All Rights Reserved.

#include "BT/BTDecorator_SBF_TagReaction.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "SBF_CharacterTraitComponent.h"

UBTDecorator_SBF_TagReaction::UBTDecorator_SBF_TagReaction()
{
	NodeName = TEXT("SBF Tag Reaction");
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

bool UBTDecorator_SBF_TagReaction::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (!TriggerTag.IsValid())
	{
		return bInverse;
	}

	AActor* OwnerActor = OwnerComp.GetOwner();
	APawn* Pawn = Cast<APawn>(OwnerActor);
	if (!Pawn)
	{
		if (AAIController* Controller = OwnerComp.GetAIOwner())
		{
			Pawn = Controller->GetPawn();
		}
	}
	if (!Pawn)
	{
		return bInverse;
	}

	const USBF_CharacterTraitComponent* TraitComponent = Pawn->FindComponentByClass<USBF_CharacterTraitComponent>();
	if (!TraitComponent)
	{
		return bInverse;
	}

	const bool bHasReaction = TraitComponent->HasReactionFor(TriggerTag);
	return bInverse ? !bHasReaction : bHasReaction;
}

FString UBTDecorator_SBF_TagReaction::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: %s reaction '%s'"),
		*GetNodeName(),
		bInverse ? TEXT("no") : TEXT("has"),
		*TriggerTag.ToString());
}

FName UBTDecorator_SBF_TagReaction::GetNodeIconName() const
{
	return FName(TEXT("BTEditor.Graph.BTNode.Decorator.Icon"));
}
