// Copyright Social Behavior Framework. All Rights Reserved.

#include "BT/BTTask_SBF_EvaluateSocial.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "SBF_ManagerSubsystem.h"
#include "SBF_NPCComponent.h"
#include "SBF_SocialGraph.h"
#include "SBF_Types.h"

UBTTask_SBF_EvaluateSocial::UBTTask_SBF_EvaluateSocial()
{
	NodeName = TEXT("SBF Evaluate Social");
}

void UBTTask_SBF_EvaluateSocial::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SBF_EvaluateSocial, TargetActorKey), AActor::StaticClass());
	IsHostileKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SBF_EvaluateSocial, IsHostileKey));
	IsAllyKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SBF_EvaluateSocial, IsAllyKey));
	RelationTagKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SBF_EvaluateSocial, RelationTagKey));

	UBlackboardData* BlackboardAsset = GetBlackboardAsset();
	if (BlackboardAsset)
	{
		TargetActorKey.CacheSelectedKeys(BlackboardAsset);
		IsHostileKey.CacheSelectedKeys(BlackboardAsset);
		IsAllyKey.CacheSelectedKeys(BlackboardAsset);
		RelationTagKey.CacheSelectedKeys(BlackboardAsset);
	}
}

EBTNodeResult::Type UBTTask_SBF_EvaluateSocial::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.GetSelectedKeyID()));
	if (!Target)
	{
		return EBTNodeResult::Failed;
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
		return EBTNodeResult::Failed;
	}

	const UWorld* World = OwnerComp.GetWorld();
	const USBF_ManagerSubsystem* Manager = World ? World->GetSubsystem<USBF_ManagerSubsystem>() : nullptr;
	if (!Manager)
	{
		return EBTNodeResult::Failed;
	}

	const ESBF_Hostility Hostility = Manager->QueryRelation(Pawn, Target);

	Blackboard->SetValueAsBool(IsHostileKey.GetSelectedKeyID(), Hostility == ESBF_Hostility::Enemy);
	Blackboard->SetValueAsBool(IsAllyKey.GetSelectedKeyID(), Hostility == ESBF_Hostility::Ally);

	FName RelationTagName = NAME_None;
	const USBF_NPCComponent* NPCComponent = Pawn->FindComponentByClass<USBF_NPCComponent>();
	const USBF_SocialGraph* Graph = NPCComponent ? NPCComponent->GetSocialGraph() : Manager->GetActiveSocialGraph();
	if (Graph)
	{
		FSBF_SocialNode NodeA, NodeB;
		if (Graph->FindNodeByActor(Pawn->GetActorGuid(), NodeA) && Graph->FindNodeByActor(Target->GetActorGuid(), NodeB))
		{
			const FGameplayTag DirectTag = Graph->GetDirectRelationTag(NodeA.NodeGuid, NodeB.NodeGuid);
			if (DirectTag.IsValid())
			{
				RelationTagName = DirectTag.GetTagName();
			}
		}
	}
	Blackboard->SetValueAsName(RelationTagKey.GetSelectedKeyID(), RelationTagName);

	return EBTNodeResult::Succeeded;
}

FString UBTTask_SBF_EvaluateSocial::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: evaluate '%s' -> '%s', '%s', '%s'"),
		*GetNodeName(),
		*TargetActorKey.SelectedKeyName.ToString(),
		*IsHostileKey.SelectedKeyName.ToString(),
		*IsAllyKey.SelectedKeyName.ToString(),
		*RelationTagKey.SelectedKeyName.ToString());
}

FName UBTTask_SBF_EvaluateSocial::GetNodeIconName() const
{
	return FName(TEXT("BTEditor.Graph.BTNode.Task.Icon"));
}
