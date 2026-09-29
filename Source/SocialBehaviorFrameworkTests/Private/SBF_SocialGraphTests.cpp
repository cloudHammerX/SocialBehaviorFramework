// Copyright Social Behavior Framework. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SBF_ManagerSubsystem.h"
#include "SBF_NPCComponent.h"
#include "SBF_SocialGraph.h"
#include "SBF_SocialRelation.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

/** Helper: creates a relation asset with the given tag and stance containers. */
namespace SBF_Test
{
	USBF_SocialRelation* MakeRelation(const FName& TagName, bool bAlly, bool bEnemy, bool bInherits)
	{
		USBF_SocialRelation* Relation = NewObject<USBF_SocialRelation>(GetTransientPackage());
		Relation->RelationTag = FGameplayTag::RequestGameplayTag(TagName, /*bErrorIfNotFound=*/false);
		Relation->bInheritsHostility = bInherits;
		if (Relation->RelationTag.IsValid())
		{
			if (bAlly) { Relation->AllyTags.AddTag(Relation->RelationTag); }
			if (bEnemy) { Relation->EnemyTags.AddTag(Relation->RelationTag); }
		}
		return Relation;
	}
}

/**
 * Social Graph hostility resolution: direct stance, inherited ally
 * ("ally of my father"), unconditional hostility propagation up the
 * hierarchy and the broken-inheritance (no-inherit) case.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSBF_SocialGraphHostilityTest, "SBF.SocialGraph.HostilityInheritance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSBF_SocialGraphHostilityTest::RunTest(const FString& Parameters)
{
	USBF_SocialRelation* FatherRelation = SBF_Test::MakeRelation(FName(TEXT("SBF.Relation.Family.Father")), /*bAlly=*/true, /*bEnemy=*/false, /*bInherits=*/true);
	USBF_SocialRelation* FriendRelation = SBF_Test::MakeRelation(FName(TEXT("SBF.Relation.Friend")), /*bAlly=*/true, /*bEnemy=*/false, /*bInherits=*/true);
	USBF_SocialRelation* EnemyRelation = SBF_Test::MakeRelation(FName(TEXT("SBF.Relation.Enemy")), /*bAlly=*/false, /*bEnemy=*/true, /*bInherits=*/true);
	USBF_SocialRelation* StrangerRelation = SBF_Test::MakeRelation(FName(TEXT("SBF.Relation.Acquaintance")), /*bAlly=*/false, /*bEnemy=*/false, /*bInherits=*/false);

	TestNotNull(TEXT("Father relation valid"), FatherRelation);
	TestNotNull(TEXT("Friend relation valid"), FriendRelation);
	TestNotNull(TEXT("Enemy relation valid"), EnemyRelation);
	TestNotNull(TEXT("Stranger relation valid"), StrangerRelation);

	USBF_SocialGraph* Graph = NewObject<USBF_SocialGraph>(GetTransientPackage());
	Graph->RelationAssets.Add(FatherRelation);
	Graph->RelationAssets.Add(FriendRelation);
	Graph->RelationAssets.Add(EnemyRelation);
	Graph->RelationAssets.Add(StrangerRelation);

	// Hierarchy: Father <- {Son, Friend, Rival}.
	const FGuid FatherNode = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Father")));
	const FGuid SonNode = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Son")), FatherNode, FatherRelation->RelationTag);
	const FGuid FriendNode = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Friend")), FatherNode, FriendRelation->RelationTag);
	const FGuid RivalNode = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Rival")), FatherNode, EnemyRelation->RelationTag);

	// Second family with a non-inheriting relation between son2 and father2.
	const FGuid Father2Node = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Father2")));
	const FGuid Son2Node = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Son2")), Father2Node, StrangerRelation->RelationTag);
	const FGuid Friend2Node = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Friend2")), Father2Node, FriendRelation->RelationTag);

	// Direct stance.
	TestEqual(TEXT("Son->Father is ally"), Graph->ResolveHostility(SonNode, FatherNode), ESBF_Hostility::Ally);
	TestEqual(TEXT("Father->Son is ally"), Graph->ResolveHostility(FatherNode, SonNode), ESBF_Hostility::Ally);
	TestEqual(TEXT("Father->Rival is enemy"), Graph->ResolveHostility(FatherNode, RivalNode), ESBF_Hostility::Enemy);

	// "Ally of my father is my ally" (inheriting chain).
	TestEqual(TEXT("Son->Friend (father's ally) is ally"), Graph->ResolveHostility(SonNode, FriendNode), ESBF_Hostility::Ally);

	// Hostility propagates up the chain to the nearest common ancestor.
	TestEqual(TEXT("Son->Rival (father's enemy) is enemy"), Graph->ResolveHostility(SonNode, RivalNode), ESBF_Hostility::Enemy);
	TestEqual(TEXT("Friend->Rival is enemy"), Graph->ResolveHostility(FriendNode, RivalNode), ESBF_Hostility::Enemy);

	// Broken inheritance: son2's relation to father2 does not inherit,
	// so father2's friend is not son2's ally.
	TestEqual(TEXT("Son2->Father2 is neutral"), Graph->ResolveHostility(Son2Node, Father2Node), ESBF_Hostility::Neutral);
	TestEqual(TEXT("Son2->Friend2 is NOT inherited"), Graph->ResolveHostility(Son2Node, Friend2Node), ESBF_Hostility::Neutral);
	TestEqual(TEXT("Father2->Friend2 is ally"), Graph->ResolveHostility(Father2Node, Friend2Node), ESBF_Hostility::Ally);

	// Unrelated families.
	TestEqual(TEXT("Son->Son2 is unknown"), Graph->ResolveHostility(SonNode, Son2Node), ESBF_Hostility::Unknown);

	// Symmetry + cache consistency.
	TestEqual(TEXT("Cache: symmetric result"), Graph->ResolveHostility(FriendNode, SonNode), ESBF_Hostility::Ally);

	// Explicit edge overrides the inference.
	USBF_SocialRelation* BossRelation = SBF_Test::MakeRelation(FName(TEXT("SBF.Relation.Work.Boss")), /*bAlly=*/false, /*bEnemy=*/false, /*bInherits=*/true);
	const FGuid BossNode = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Boss")));
	const FGuid WorkerNode = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Worker")), BossNode, BossRelation->RelationTag);
	const FGuid Rival2Node = Graph->AddNode(FGuid::NewGuid(), FName(TEXT("Rival2")));
	Graph->RelationAssets.Add(BossRelation);
	const FGuid EdgeGuid = Graph->Link(BossNode, Rival2Node, EnemyRelation, /*bBidirectional=*/true);
	TestTrue(TEXT("Edge created"), EdgeGuid.IsValid());

	// Worker->Rival2: worker->boss (neutral) then boss->rival2 explicit enemy edge.
	TestEqual(TEXT("Worker->Rival2 (boss's enemy) is enemy"), Graph->ResolveHostility(WorkerNode, Rival2Node), ESBF_Hostility::Enemy);

	// Structural validation must pass on this graph.
	TArray<FText> Errors;
	TestTrue(TEXT("Graph validates"), Graph->Validate(Errors));

	// Mutations invalidate the cache (result stays correct after reparent).
	TestTrue(TEXT("Reparent Son2 under Father"), Graph->SetParent(Son2Node, FatherNode, FatherRelation->RelationTag));
	TestEqual(TEXT("Son2->Friend after reparent is ally"), Graph->ResolveHostility(Son2Node, FriendNode), ESBF_Hostility::Ally);

	// Actor-based query through the manager facade.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld=*/false);
	if (!World)
	{
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeNewWorld(UWorld::InitializationValues()
		.AllowAudioPlayback(false)
		.CreatePhysicsScene(false)
		.RequiresHitProxies(false)
		.CreateNavigation(false)
		.CreateAISystem(false)
		.ShouldSimulatePhysics(false)
		.SetTransactional(false));

	USBF_ManagerSubsystem* Manager = World->GetSubsystem<USBF_ManagerSubsystem>();
	TestNotNull(TEXT("Manager subsystem created"), Manager);

	AActor* ActorSon = World->SpawnActor<AActor>();
	AActor* ActorFather = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Actor spawned"), ActorSon);
	TestNotNull(TEXT("Actor spawned"), ActorFather);

	Graph->BindNodeToActor(SonNode, ActorSon->GetActorGuid());
	Graph->BindNodeToActor(FatherNode, ActorFather->GetActorGuid());

	USBF_NPCComponent* SonComponent = NewObject<USBF_NPCComponent>(ActorSon);
	USBF_NPCComponent* FatherComponent = NewObject<USBF_NPCComponent>(ActorFather);
	SonComponent->SocialGraph = Graph;
	FatherComponent->SocialGraph = Graph;

	Manager->RegisterNPC(ActorSon, SonComponent);
	Manager->RegisterNPC(ActorFather, FatherComponent);

	TestEqual(TEXT("QueryRelation(ActorSon, ActorFather) is ally"), Manager->QueryRelation(ActorSon, ActorFather), ESBF_Hostility::Ally);
	TestEqual(TEXT("QueryRelation self is neutral"), Manager->QueryRelation(ActorSon, ActorSon), ESBF_Hostility::Neutral);

	Manager->UnregisterNPC(ActorSon);
	Manager->UnregisterNPC(ActorFather);
	TestEqual(TEXT("QueryRelation after unregister is unknown"), Manager->QueryRelation(ActorSon, ActorFather), ESBF_Hostility::Unknown);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld=*/false);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
