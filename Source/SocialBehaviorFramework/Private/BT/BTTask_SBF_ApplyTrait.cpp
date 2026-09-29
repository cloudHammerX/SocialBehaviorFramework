// Copyright Social Behavior Framework. All Rights Reserved.

#include "BT/BTTask_SBF_ApplyTrait.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"
#include "SBF_CharacterTraitComponent.h"
#include "SBF_Log.h"
#include "SBF_Types.h"

UBTTask_SBF_ApplyTrait::UBTTask_SBF_ApplyTrait()
{
	NodeName = TEXT("SBF Apply Trait");
	bCreateNodeInstance = false;
}

void UBTTask_SBF_ApplyTrait::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	ReactionTagKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SBF_ApplyTrait, ReactionTagKey));

	if (UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		ReactionTagKey.CacheSelectedKeys(BlackboardAsset);
	}
}

EBTNodeResult::Type UBTTask_SBF_ApplyTrait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTTask_SBF_ApplyTraitMemory* Memory = CastInstanceNodeMemory<FBTTask_SBF_ApplyTraitMemory>(NodeMemory);
	Memory->bSubtreeRunning = false;
	Memory->PushedInstanceIdx = INDEX_NONE;

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
		return EBTNodeResult::Failed;
	}

	USBF_CharacterTraitComponent* TraitComponent = Pawn->FindComponentByClass<USBF_CharacterTraitComponent>();
	if (!TraitComponent)
	{
		UE_LOG(LogSBF, Verbose, TEXT("BTTask_SBF_ApplyTrait: '%s' has no USBF_CharacterTraitComponent."), *Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	FSBF_TraitReaction Reaction;
	if (!TraitComponent->GetBestPendingReaction(Reaction))
	{
		return EBTNodeResult::Failed;
	}

	TraitComponent->ConsumeReaction(Reaction.TriggerTag);
	WriteReactionKeys(OwnerComp, Reaction);

	if (Reaction.OverrideSubtree)
	{
		UBehaviorTree& Subtree = *Reaction.OverrideSubtree;

		// Push the override subtree on the execution stack, rooted at this task
		// node (mirrors the engine's UBTTask_RunBehavior mechanism).
		FBehaviorTreeInstance SubtreeInstance(Subtree, *this, OwnerComp);
		const bool bPushed = OwnerComp.PushInstance(SubtreeInstance);
		if (!bPushed)
		{
			UE_LOG(LogSBF, Warning, TEXT("BTTask_SBF_ApplyTrait: failed to push subtree '%s'."), *Subtree.GetName());
			return EBTNodeResult::Failed;
		}

		Memory->bSubtreeRunning = true;
		Memory->PushedInstanceIdx = OwnerComp.GetInstanceStackNum() - 1;
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

void UBTTask_SBF_ApplyTrait::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FBTTask_SBF_ApplyTraitMemory* Memory = CastInstanceNodeMemory<FBTTask_SBF_ApplyTraitMemory>(NodeMemory);
	Memory->bSubtreeRunning = false;
	Memory->PushedInstanceIdx = INDEX_NONE;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_SBF_ApplyTrait::OnTaskAborted(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTTask_SBF_ApplyTraitMemory* Memory = CastInstanceNodeMemory<FBTTask_SBF_ApplyTraitMemory>(NodeMemory);

	if (Memory->bSubtreeRunning && Memory->PushedInstanceIdx != INDEX_NONE && OwnerComp.GetInstanceStackNum() > Memory->PushedInstanceIdx)
	{
		OwnerComp.StopTree(Memory->PushedInstanceIdx);
	}

	Memory->bSubtreeRunning = false;
	Memory->PushedInstanceIdx = INDEX_NONE;
}

uint16 UBTTask_SBF_ApplyTrait::GetInstanceMemorySize() const
{
	return sizeof(FBTTask_SBF_ApplyTraitMemory);
}

void UBTTask_SBF_ApplyTrait::WriteReactionKeys(UBehaviorTreeComponent& OwnerComp, const FSBF_TraitReaction& Reaction) const
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	Blackboard->SetValueAsName(ReactionTagKey.GetSelectedKeyID(), Reaction.TriggerTag.GetTagName());

	for (const FName& KeyName : Reaction.BlackboardKeysToSet)
	{
		const FBlackboard::FKey KeyID = Blackboard->GetKeyID(KeyName);
		if (KeyID == FBlackboard::InvalidKey)
		{
			UE_LOG(LogSBF, Warning, TEXT("BTTask_SBF_ApplyTrait: blackboard key '%s' does not exist."), *KeyName.ToString());
			continue;
		}

		const UBlackboardKeyType* KeyType = Blackboard->GetKeyType(KeyID);
		if (KeyType && KeyType->IsA<UBlackboardKeyType_Bool>())
		{
			Blackboard->SetValueAsBool(KeyID, true);
		}
		else if (KeyType && KeyType->IsA<UBlackboardKeyType_Name>())
		{
			Blackboard->SetValueAsName(KeyID, Reaction.TriggerTag.GetTagName());
		}
		else
		{
			UE_LOG(LogSBF, Warning, TEXT("BTTask_SBF_ApplyTrait: key '%s' is neither bool nor name; skipped."), *KeyName.ToString());
		}
	}
}

FString UBTTask_SBF_ApplyTrait::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: apply best pending trait reaction -> '%s'"),
		*GetNodeName(),
		*ReactionTagKey.SelectedKeyName.ToString());
}

FName UBTTask_SBF_ApplyTrait::GetNodeIconName() const
{
	return FName(TEXT("BTEditor.Graph.BTNode.Task.Icon"));
}
