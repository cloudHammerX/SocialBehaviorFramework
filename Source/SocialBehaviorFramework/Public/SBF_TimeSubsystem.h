// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/TickableWorldSubsystem.h"
#include "Engine/World.h"
#include "SBF_Types.h"
#include "SBF_TimeSubsystem.generated.h"

/**
 * Game time subsystem (world scoped).
 *
 * Advances a simulated clock configured through USBF_DeveloperSettings and
 * broadcasts per-minute / per-hour / per-day delegates instead of polling.
 * NPCs subscribe through OnMinuteTick (see USBF_NPCComponent); no per-NPC
 * timers are used.
 */
UCLASS()
class SOCIALBEHAVIORFRAMEWORK_API USBF_TimeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Fired every game minute with the new time snapshot. */
	FOnGameTimeChanged OnMinuteTick;

	/** Fired when the game hour changes. */
	FOnGameTimeChanged OnHourTick;

	/** Fired when the simulated day rolls over (00:00). */
	FOnGameTimeChanged OnDayChanged;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// ~UTickableWorldSubsystem
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override;

	/** @return the current game time snapshot. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	FSBF_GameTime GetGameTime() const;

	/** @return the current weekday FName ("Monday" .. "Sunday"). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	FName GetWeekday() const;

	/** @return 0-based weekday index (0 == Monday). */
	UFUNCTION(BlueprintPure, Category = "SBF")
	int32 GetWeekdayIndex() const;

	/** @return true when the current day is a weekend day. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool IsWeekend() const;

	/** @return the current time scale. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	float GetTimeScale() const { return TimeScale; }

	/** @return true when the clock is paused. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool IsPaused() const { return bPaused; }

	/** Sets the time scale (0 freezes time). */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void SetTimeScale(float NewScale);

	/** Pauses / resumes the clock. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void SetPaused(bool bNewPaused);

	/**
	 * Hard-sets the game time (0-based day index).
	 * No retroactive delegate broadcasts are emitted.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void SetGameTime(int32 Hour, int32 Minute, int32 DayIndex);

	/** Re-reads USBF_DeveloperSettings (e.g. after a project settings change). */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void ReloadSettings();

private:
	/** Absolute simulated minute counter (since day 0, 00:00). */
	double AccumulatedGameMinutes = 0.0;

	/** Last broadcast minute boundary (avoids duplicate broadcasts). */
	int64 LastBroadcastTotalMinutes = 0;

	float TimeScale = 1.0f;
	float RealSecondsPerGameMinute = 1.0f;
	bool bPaused = false;
	bool bEnableWeekdays = true;
	TArray<FName> WeekendDays;

	FSBF_GameTime MakeTime(int64 TotalMinutes) const;
	void BroadcastUpTo(int64 NewTotalMinutes);
};
