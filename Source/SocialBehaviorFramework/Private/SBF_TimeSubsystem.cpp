// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_TimeSubsystem.h"

#include "Engine/World.h"
#include "SBF_DeveloperSettings.h"
#include "SBF_Log.h"
#include "Stats/Stats.h"

void USBF_TimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadSettings();
}

void USBF_TimeSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool USBF_TimeSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::EditorPreview;
}

TStatId USBF_TimeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USBF_TimeSubsystem, STATGROUP_Tickables);
}

ETickableTickType USBF_TimeSubsystem::GetTickableTickType() const
{
	return IsTemplate() ? ETickableTickType::Never : ETickableTickType::Conditional;
}

bool USBF_TimeSubsystem::IsTickable() const
{
	return !bPaused && !IsTemplate() && IsValid(GetWorld()) && GetWorld()->IsGameWorld();
}

void USBF_TimeSubsystem::Tick(float DeltaTime)
{
	if (bPaused || TimeScale <= 0.0f || RealSecondsPerGameMinute <= 0.0f)
	{
		return;
	}

	// Game minutes advanced per real second = 1 / RealSecondsPerGameMinute.
	const double MinutesPerRealSecond = 1.0 / static_cast<double>(RealSecondsPerGameMinute);
	AccumulatedGameMinutes += static_cast<double>(DeltaTime) * static_cast<double>(TimeScale) * MinutesPerRealSecond;

	const int64 NewTotal = FMath::FloorToInt64(AccumulatedGameMinutes);
	if (NewTotal > LastBroadcastTotalMinutes)
	{
		BroadcastUpTo(NewTotal);
	}
}

FSBF_GameTime USBF_TimeSubsystem::GetGameTime() const
{
	return MakeTime(FMath::FloorToInt64(AccumulatedGameMinutes));
}

FName USBF_TimeSubsystem::GetWeekday() const
{
	return GetGameTime().Weekday;
}

int32 USBF_TimeSubsystem::GetWeekdayIndex() const
{
	return GetGameTime().GetWeekdayIndex();
}

bool USBF_TimeSubsystem::IsWeekend() const
{
	return bEnableWeekdays && GetGameTime().bIsWeekend;
}

void USBF_TimeSubsystem::SetTimeScale(float NewScale)
{
	TimeScale = FMath::Max(0.0f, NewScale);
}

void USBF_TimeSubsystem::SetPaused(bool bNewPaused)
{
	bPaused = bNewPaused;
	UE_LOG(LogSBF, Verbose, TEXT("SBF time %s."), bPaused ? TEXT("paused") : TEXT("resumed"));
}

void USBF_TimeSubsystem::SetGameTime(int32 Hour, int32 Minute, int32 DayIndex)
{
	Hour = FMath::Clamp(Hour, 0, 23);
	Minute = FMath::Clamp(Minute, 0, 59);
	DayIndex = FMath::Max(0, DayIndex);

	const int64 TotalMinutes = static_cast<int64>(DayIndex) * 24 * 60 + Hour * 60 + Minute;
	AccumulatedGameMinutes = static_cast<double>(TotalMinutes);
	LastBroadcastTotalMinutes = TotalMinutes;

	UE_LOG(LogSBF, Verbose, TEXT("SBF time set to day %d %02d:%02d."), DayIndex, Hour, Minute);
}

void USBF_TimeSubsystem::ReloadSettings()
{
	const USBF_DeveloperSettings& Settings = USBF_DeveloperSettings::Get();

	TimeScale = Settings.TimeScale;
	RealSecondsPerGameMinute = Settings.RealSecondsPerGameMinute;
	bPaused = Settings.bPaused;
	bEnableWeekdays = Settings.bEnableWeekdays;
	WeekendDays = Settings.WeekendDays;

	// Re-baseline the clock on the configured start time (only if not advanced yet).
	if (LastBroadcastTotalMinutes == 0 && AccumulatedGameMinutes == 0.0)
	{
		const int64 StartTotal = static_cast<int64>(Settings.StartDay) * 24 * 60 + Settings.StartHour * 60 + Settings.StartMinute;
		AccumulatedGameMinutes = static_cast<double>(StartTotal);
		LastBroadcastTotalMinutes = StartTotal;
	}
}

FSBF_GameTime USBF_TimeSubsystem::MakeTime(int64 TotalMinutes) const
{
	return FSBF_GameTime::FromTotalMinutes(TotalMinutes, WeekendDays);
}

void USBF_TimeSubsystem::BroadcastUpTo(int64 NewTotalMinutes)
{
	while (LastBroadcastTotalMinutes < NewTotalMinutes)
	{
		++LastBroadcastTotalMinutes;
		const FSBF_GameTime Time = MakeTime(LastBroadcastTotalMinutes);

		OnMinuteTick.Broadcast(Time);

		if (Time.Minute == 0)
		{
			OnHourTick.Broadcast(Time);
		}
		if (Time.Hour == 0 && Time.Minute == 0)
		{
			OnDayChanged.Broadcast(Time);
		}
	}
}
