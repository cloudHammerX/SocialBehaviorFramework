// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_NPCComponent.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SBF_BehaviorMap.h"
#include "SBF_CharacterTraitComponent.h"
#include "SBF_Log.h"
#include "SBF_ManagerSubsystem.h"
#include "SBF_SocialGraph.h"
#include "SBF_SocialRelation.h"
#include "SBF_TimeSubsystem.h"
#include "Blueprint/UserWidget.h"

USBF_NPCComponent::USBF_NPCComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USBF_NPCComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}

	if (USBF_ManagerSubsystem* Manager = World->GetSubsystem<USBF_ManagerSubsystem>())
	{
		Manager->RegisterNPC(GetOwner(), this);
	}

	// Ensure a trait component exists so reactions can be routed to it.
	GetTraitComponent();

	// Event-driven schedule updates - no per-NPC timers.
	if (USBF_TimeSubsystem* TimeSubsystem = World->GetSubsystem<USBF_TimeSubsystem>())
	{
		MinuteTickHandle = TimeSubsystem->OnMinuteTick.AddUObject(this, &USBF_NPCComponent::HandleMinuteTick);
	}

	ReevaluateSchedule(/*bForce=*/true);
	SpawnRelationWidget();
}

void USBF_NPCComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (USBF_TimeSubsystem* TimeSubsystem = World->GetSubsystem<USBF_TimeSubsystem>())
		{
			TimeSubsystem->OnMinuteTick.Remove(MinuteTickHandle);
		}
		if (USBF_ManagerSubsystem* Manager = World->GetSubsystem<USBF_ManagerSubsystem>())
		{
			Manager->UnregisterNPC(GetOwner());
		}
	}

	MinuteTickHandle.Reset();
	Super::EndPlay(EndPlayReason);
}

void USBF_NPCComponent::HandleMinuteTick(const FSBF_GameTime& GameTime)
{
	ReevaluateSchedule(/*bForce=*/false);
}

void USBF_NPCComponent::ReevaluateSchedule(bool bForce)
{
	const USBF_BehaviorMap* Map = BehaviorMap;
	if (!Map)
	{
		if (bForce)
		{
			UE_LOG(LogSBF, Verbose, TEXT("SBF: NPC '%s' has no BehaviorMap assigned, schedule skipped."), *GetNameSafe(GetOwner()));
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	FSBF_GameTime Now;
	const USBF_TimeSubsystem* TimeSubsystem = World->GetSubsystem<USBF_TimeSubsystem>();
	if (TimeSubsystem)
	{
		Now = TimeSubsystem->GetGameTime();
	}
	else
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF: time subsystem missing in world '%s'."), *GetNameSafe(World));
		return;
	}

	ESBF_ActivityType Activity = ESBF_ActivityType::None;
	FVector Location = FVector::ZeroVector;
	Map->ResolveCurrentActivity(Now, Activity, Location);

	if (!bForce && Activity == CachedActivity && Location.Equals(CachedTargetLocation, 1.0f))
	{
		return;
	}

	CachedActivity = Activity;
	CachedTargetLocation = Location;

	WriteBlackboard();
	OnScheduleChanged.Broadcast(Activity, Location);

	// Optional automatic movement (Home -> Work -> Leisure).
	if (bAutoMove)
	{
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (AAIController* Controller = Cast<AAIController>(Pawn->GetController()))
			{
				Controller->MoveToLocation(Location, MoveAcceptanceRadius, /*bStopOnOverlap=*/true);
			}
		}
	}

	UE_LOG(LogSBF, Verbose, TEXT("SBF: NPC '%s' schedule -> %d @ %s"), *GetNameSafe(GetOwner()), static_cast<int32>(Activity), *Location.ToString());
}

ESBF_Hostility USBF_NPCComponent::QueryRelationTowards(const AActor* Other) const
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return ESBF_Hostility::Unknown;
	}
	const USBF_ManagerSubsystem* Manager = World->GetSubsystem<USBF_ManagerSubsystem>();
	if (!Manager)
	{
		return ESBF_Hostility::Unknown;
	}
	return Manager->QueryRelation(GetOwner(), Other);
}

USBF_CharacterTraitComponent* USBF_NPCComponent::GetTraitComponent() const
{
	if (CachedTraitComponent.IsValid())
	{
		return CachedTraitComponent.Get();
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	USBF_CharacterTraitComponent* TraitComponent = Owner->FindComponentByClass<USBF_CharacterTraitComponent>();
	if (!TraitComponent)
	{
		TraitComponent = NewObject<USBF_CharacterTraitComponent>(Owner, USBF_CharacterTraitComponent::StaticClass(), FName(TEXT("SBF_CharacterTrait")));
		if (TraitComponent)
		{
			TraitComponent->RegisterComponent();
			Owner->AddInstanceComponent(TraitComponent);
		}
	}

	if (TraitComponent && !TraitComponent->CharacterTrait)
	{
		TraitComponent->CharacterTrait = CharacterTrait;
	}

	CachedTraitComponent = TraitComponent;
	return TraitComponent;
}

UBlackboardComponent* USBF_NPCComponent::GetCachedBlackboard() const
{
	if (CachedBlackboard.IsValid())
	{
		return CachedBlackboard.Get();
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return nullptr;
	}

	const AAIController* Controller = Cast<AAIController>(Pawn->GetController());
	if (!Controller)
	{
		return nullptr;
	}

	UBlackboardComponent* Blackboard = Controller->GetBlackboardComponent();
	CachedBlackboard = Blackboard;
	return Blackboard;
}

void USBF_NPCComponent::WriteBlackboard()
{
	UBlackboardComponent* Blackboard = GetCachedBlackboard();
	if (!Blackboard)
	{
		return;
	}

	const FBlackboard::FKey LocationKey = Blackboard->GetKeyID(TargetLocationKey);
	if (LocationKey != FBlackboard::InvalidKey)
	{
		Blackboard->SetValueAsVector(LocationKey, CachedTargetLocation);
	}

	const FBlackboard::FKey ActivityKeyId = Blackboard->GetKeyID(ActivityKey);
	if (ActivityKeyId != FBlackboard::InvalidKey)
	{
		Blackboard->SetValueAsName(ActivityKeyId, StaticEnum<ESBF_ActivityType>()->GetNameByValue(static_cast<int64>(CachedActivity)));
	}
}

void USBF_NPCComponent::SpawnRelationWidget()
{
	if (!bSpawnRelationWidget || RelationWidgetComponent)
	{
		return;
	}

	const USBF_SocialRelation* Relation = ResolveOwnRelation();
	if (!Relation || !Relation->NodeWidgetClass)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	UWidgetComponent* WidgetComponent = NewObject<UWidgetComponent>(Owner, UWidgetComponent::StaticClass(), FName(TEXT("SBF_RelationWidget")));
	if (!WidgetComponent)
	{
		return;
	}

	WidgetComponent->SetWidgetClass(Relation->NodeWidgetClass);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawSize(FVector2D(240.0f, 64.0f));
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	WidgetComponent->SetVisibility(true);
	WidgetComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	WidgetComponent->RegisterComponent();

	RelationWidgetComponent = WidgetComponent;
}

const USBF_SocialRelation* USBF_NPCComponent::ResolveOwnRelation() const
{
	const USBF_SocialGraph* Graph = SocialGraph;
	if (!Graph)
	{
		return nullptr;
	}

	const AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	FSBF_SocialNode Node;
	if (!Graph->FindNodeByActor(Owner->GetActorGuid(), Node))
	{
		return nullptr;
	}

	return Graph->FindRelationAsset(Node.RelationTag);
}

FGameplayTag USBF_NPCComponent::GetOwnRelationTag() const
{
	const USBF_SocialRelation* Relation = ResolveOwnRelation();
	return Relation ? Relation->RelationTag : FGameplayTag();
}
