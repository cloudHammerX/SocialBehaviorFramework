// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BTService_SBF_TimeSync.generated.h"

/**
 * Behavior Tree service: periodically syncs the game time (from
 * USBF_TimeSubsystem) into the blackboard:
 *   GameHourKey  - current game hour (0-23),
 *   WeekdayKey   - 0-based weekday index (0 == Monday),
 *   IsWeekendKey - true on weekend days.
 *
 * Values are also written on search start, so branches can react immediately.
 */
UCLASS(Category = "SBF")
class SOCIALBEHAVIORFRAMEWORK_API UBTService_SBF_TimeSync : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SBF_TimeSync();

	/** Output key: game hour. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector GameHourKey;

	/** Output key: weekday index (0 == Monday). */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WeekdayKey;

	/** Output key: true on weekends. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsWeekendKey;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnSearchStart(FBehaviorTreeSearchData& SearchData) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual FName GetNodeIconName() const override;

private:
	void SyncTime(UBehaviorTreeComponent& OwnerComp) const;
};
