// Copyright Social Behavior Framework. All Rights Reserved.

#include "BT/BTTask_SBF_UpdateSchedule.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SBF_Log.h"
#include "SBF_NPCComponent.h"

UBTTask_SBF_UpdateSchedule::UBTTask_SBF_UpdateSchedule()
{
	NodeName = TEXT("SBF Update Schedule");
}

void UBTTask_SBF_UpdateSchedule::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SBF_UpdateSchedule, TargetLocationKey));
	ActivityKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SBF_UpdateSchedule, ActivityKey));

	UBlackboardData* BlackboardAsset = GetBlackboardAsset();
	if (BlackboardAsset)
	{
		TargetLocationKey.CacheSelectedKeys(BlackboardAsset);
		ActivityKey.CacheSelectedKeys(BlackboardAsset);
	}
}

EBTNodeResult::Type UBTTask_SBF_UpdateSchedule::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
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

	USBF_NPCComponent* NPCComponent = Pawn->FindComponentByClass<USBF_NPCComponent>();
	if (!NPCComponent)
	{
		UE_LOG(LogSBF, Verbose, TEXT("BTTask_SBF_UpdateSchedule: '%s' has no USBF_NPCComponent."), *Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	NPCComponent->ReevaluateSchedule(/*bForce=*/true);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		Blackboard->SetValueAsVector(TargetLocationKey.GetSelectedKeyID(), NPCComponent->GetTargetLocation());
		Blackboard->SetValueAsName(ActivityKey.GetSelectedKeyID(), StaticEnum<ESBF_ActivityType>()->GetNameByValue(static_cast<int64>(NPCComponent->GetCurrentActivity())));
	}

	return EBTNodeResult::Succeeded;
}

FString UBTTask_SBF_UpdateSchedule::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: re-evaluate schedule -> '%s', '%s'"),
		*GetNodeName(),
		*TargetLocationKey.SelectedKeyName.ToString(),
		*ActivityKey.SelectedKeyName.ToString());
}

FName UBTTask_SBF_UpdateSchedule::GetNodeIconName() const
{
	return FName(TEXT("BTEditor.Graph.BTNode.Task.Icon"));
}
