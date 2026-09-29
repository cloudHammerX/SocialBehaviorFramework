// Copyright Social Behavior Framework. All Rights Reserved.

#include "BT/BTService_SBF_TimeSync.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Engine/World.h"
#include "SBF_TimeSubsystem.h"

UBTService_SBF_TimeSync::UBTService_SBF_TimeSync()
{
	NodeName = TEXT("SBF Time Sync");

	Interval = 0.5f;
	RandomDeviation = 0.1f;
	bNotifyBecomeRelevant = true;
}

void UBTService_SBF_TimeSync::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	GameHourKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SBF_TimeSync, GameHourKey));
	WeekdayKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SBF_TimeSync, WeekdayKey));
	IsWeekendKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SBF_TimeSync, IsWeekendKey));

	if (UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		GameHourKey.CacheSelectedKeys(BlackboardAsset);
		WeekdayKey.CacheSelectedKeys(BlackboardAsset);
		IsWeekendKey.CacheSelectedKeys(BlackboardAsset);
	}
}

void UBTService_SBF_TimeSync::OnSearchStart(FBehaviorTreeSearchData& SearchData)
{
	Super::OnSearchStart(SearchData);
	SyncTime(SearchData.OwnerComp);
}

void UBTService_SBF_TimeSync::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	SyncTime(OwnerComp);
}

void UBTService_SBF_TimeSync::SyncTime(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	const UWorld* World = OwnerComp.GetWorld();
	const USBF_TimeSubsystem* TimeSubsystem = World ? World->GetSubsystem<USBF_TimeSubsystem>() : nullptr;
	if (!TimeSubsystem)
	{
		return;
	}

	const FSBF_GameTime Time = TimeSubsystem->GetGameTime();
	Blackboard->SetValueAsInt(GameHourKey.GetSelectedKeyID(), Time.Hour);
	Blackboard->SetValueAsInt(WeekdayKey.GetSelectedKeyID(), Time.GetWeekdayIndex());
	Blackboard->SetValueAsBool(IsWeekendKey.GetSelectedKeyID(), TimeSubsystem->IsWeekend());
}

FString UBTService_SBF_TimeSync::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: sync game time -> '%s', '%s', '%s' (interval %.2fs)"),
		*GetNodeName(),
		*GameHourKey.SelectedKeyName.ToString(),
		*WeekdayKey.SelectedKeyName.ToString(),
		*IsWeekendKey.SelectedKeyName.ToString(),
		Interval);
}

FName UBTService_SBF_TimeSync::GetNodeIconName() const
{
	return FName(TEXT("BTEditor.Graph.BTNode.Service.Icon"));
}
