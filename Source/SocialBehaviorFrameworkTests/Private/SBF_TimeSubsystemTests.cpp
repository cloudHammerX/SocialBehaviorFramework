// Copyright Social Behavior Framework. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "SBF_TimeSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Time subsystem: minute/hour/day delegate boundaries, day rollover and
 * weekend detection. The clock is driven manually through Tick() with
 * controlled deltas (RealSecondsPerGameMinute defaults to 1.0).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSBF_TimeSubsystemTest, "SBF.TimeSubsystem.DayWeekBoundaries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSBF_TimeSubsystemTest::RunTest(const FString& Parameters)
{
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

	USBF_TimeSubsystem* TimeSubsystem = World->GetSubsystem<USBF_TimeSubsystem>();
	TestNotNull(TEXT("Time subsystem created"), TimeSubsystem);

	if (TimeSubsystem)
	{
		// Baseline at day 0, 00:00 with default settings (1 real second = 1 game minute).
		TimeSubsystem->SetTimeScale(1.0f);
		TimeSubsystem->SetPaused(false);
		TimeSubsystem->SetGameTime(0, 0, 0);

		int32 MinuteTicks = 0;
		int32 HourTicks = 0;
		int32 DayChanges = 0;
		const FDelegateHandle MinuteHandle = TimeSubsystem->OnMinuteTick.AddLambda([&MinuteTicks](const FSBF_GameTime&) { ++MinuteTicks; });
		const FDelegateHandle HourHandle = TimeSubsystem->OnHourTick.AddLambda([&HourTicks](const FSBF_GameTime&) { ++HourTicks; });
		const FDelegateHandle DayHandle = TimeSubsystem->OnDayChanged.AddLambda([&DayChanges](const FSBF_GameTime&) { ++DayChanges; });

		// 120 real seconds -> 120 game minutes -> 2 hour boundaries, no day rollover.
		TimeSubsystem->Tick(120.0f);
		TestEqual(TEXT("120 minute ticks"), MinuteTicks, 120);
		TestEqual(TEXT("2 hour ticks"), HourTicks, 2);
		TestEqual(TEXT("No day change"), DayChanges, 0);

		{
			const FSBF_GameTime Time = TimeSubsystem->GetGameTime();
			TestEqual(TEXT("Hour is 2"), Time.Hour, 2);
			TestEqual(TEXT("Minute is 0"), Time.Minute, 0);
			TestEqual(TEXT("Day is 1"), Time.Day, 1);
			TestEqual(TEXT("Weekday is Monday"), Time.Weekday, FName(TEXT("Monday")));
		}

		// Pause freezes the clock.
		TimeSubsystem->SetPaused(true);
		const int32 TicksBeforePause = MinuteTicks;
		TimeSubsystem->Tick(60.0f);
		TestEqual(TEXT("Paused clock does not tick"), MinuteTicks, TicksBeforePause);
		TimeSubsystem->SetPaused(false);

		// Time scale 0 also freezes.
		TimeSubsystem->SetTimeScale(0.0f);
		TimeSubsystem->Tick(60.0f);
		TestEqual(TEXT("Zero scale does not tick"), MinuteTicks, TicksBeforePause);
		TimeSubsystem->SetTimeScale(1.0f);

		// Day boundary: 23:58 + 120 real seconds (120 game minutes) -> 01:58 next day.
		MinuteTicks = 0;
		HourTicks = 0;
		DayChanges = 0;
		TimeSubsystem->SetGameTime(23, 58, 0);
		TimeSubsystem->Tick(120.0f);
		TestEqual(TEXT("Boundary: 120 minute ticks"), MinuteTicks, 120);
		TestEqual(TEXT("Boundary: 2 hour ticks (00:00, 01:00)"), HourTicks, 2);
		TestEqual(TEXT("Boundary: 1 day change"), DayChanges, 1);

		{
			const FSBF_GameTime Time = TimeSubsystem->GetGameTime();
			TestEqual(TEXT("Rollover hour is 1"), Time.Hour, 1);
			TestEqual(TEXT("Rollover minute is 58"), Time.Minute, 58);
			TestEqual(TEXT("Rollover day is 2"), Time.Day, 2);
			TestEqual(TEXT("Rollover weekday is Tuesday"), Time.Weekday, FName(TEXT("Tuesday")));
		}

		// Weekend detection: day index 5 = Saturday.
		TimeSubsystem->SetGameTime(12, 0, 5);
		TestTrue(TEXT("Saturday is weekend"), TimeSubsystem->IsWeekend());
		TestEqual(TEXT("Weekday index is 5"), TimeSubsystem->GetWeekdayIndex(), 5);

		// Friday (day index 4) is not a weekend day.
		TimeSubsystem->SetGameTime(12, 0, 4);
		TestFalse(TEXT("Friday is not weekend"), TimeSubsystem->IsWeekend());

		TimeSubsystem->OnMinuteTick.Remove(MinuteHandle);
		TimeSubsystem->OnHourTick.Remove(HourHandle);
		TimeSubsystem->OnDayChanged.Remove(DayHandle);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld=*/false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
