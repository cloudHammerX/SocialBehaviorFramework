// Copyright Social Behavior Framework. All Rights Reserved.

#include "BT/BTDecorator_SBF_IsHostile.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "SBF_ManagerSubsystem.h"
#include "SBF_Types.h"

UBTDecorator_SBF_IsHostile::UBTDecorator_SBF_IsHostile()
{
	NodeName = TEXT("SBF Is Hostile");
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

void UBTDecorator_SBF_IsHostile::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_SBF_IsHostile, TargetActorKey), AActor::StaticClass());

	if (UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		TargetActorKey.CacheSelectedKeys(BlackboardAsset);
	}
}

bool UBTDecorator_SBF_IsHostile::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return Evaluate(OwnerComp);
}

bool UBTDecorator_SBF_IsHostile::Evaluate(const UBehaviorTreeComponent& OwnerComp) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return false;
	}

	// Resolve the controlled pawn (the brain component's owner may be the AI controller).
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
		return false;
	}

	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.GetSelectedKeyID()));
	if (!Target)
	{
		return false;
	}

	const UWorld* World = OwnerComp.GetWorld();
	const USBF_ManagerSubsystem* Manager = World ? World->GetSubsystem<USBF_ManagerSubsystem>() : nullptr;
	if (!Manager)
	{
		return false;
	}

	return Manager->QueryRelation(Pawn, Target) == ESBF_Hostility::Enemy;
}

FString UBTDecorator_SBF_IsHostile::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: target '%s' is hostile"), *GetNodeName(), *TargetActorKey.SelectedKeyName.ToString());
}

FName UBTDecorator_SBF_IsHostile::GetNodeIconName() const
{
	return FName(TEXT("BTEditor.Graph.BTNode.Decorator.Icon"));
}
