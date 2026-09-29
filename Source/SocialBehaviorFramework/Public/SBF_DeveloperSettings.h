// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SBF_DeveloperSettings.generated.h"

class USBF_SocialGraph;
class USBF_BehaviorMap;
class USBF_CharacterTrait;

/**
 * Project settings of the Social Behavior Framework (Project Settings ->
 * Game -> Social Behavior Framework).
 *
 * Time settings drive USBF_TimeSubsystem; the default asset pointers are the
 * fallback assets used by USBF_ManagerSubsystem when an NPC does not override
 * them on its USBF_NPCComponent.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Social Behavior Framework"))
class SOCIALBEHAVIORFRAMEWORK_API USBF_DeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USBF_DeveloperSettings();

	/** Real seconds per one game minute (1.0 = 1 game minute per real second). */
	UPROPERTY(config, EditAnywhere, Category = "Time", meta = (ClampMin = "0.001"))
	float RealSecondsPerGameMinute = 1.0f;

	/** Hour the simulated day starts at. */
	UPROPERTY(config, EditAnywhere, Category = "Time", meta = (ClampMin = "0", ClampMax = "23"))
	int32 StartHour = 6;

	/** Minute the simulated day starts at. */
	UPROPERTY(config, EditAnywhere, Category = "Time", meta = (ClampMin = "0", ClampMax = "59"))
	int32 StartMinute = 0;

	/** 0-based start day index (0 = Monday). */
	UPROPERTY(config, EditAnywhere, Category = "Time", meta = (ClampMin = "0"))
	int32 StartDay = 0;

	/** When true, the weekend days listed below skip work schedules. */
	UPROPERTY(config, EditAnywhere, Category = "Time")
	bool bEnableWeekdays = true;

	/** Days treated as weekend (FName form: "Saturday", "Sunday" ...). */
	UPROPERTY(config, EditAnywhere, Category = "Time", meta = (EditCondition = "bEnableWeekdays"))
	TArray<FName> WeekendDays = { FName(TEXT("Saturday")), FName(TEXT("Sunday")) };

	/** Global time multiplier applied on top of RealSecondsPerGameMinute. */
	UPROPERTY(config, EditAnywhere, Category = "Time", meta = (ClampMin = "0.0"))
	float TimeScale = 1.0f;

	/** When true, game time is frozen. */
	UPROPERTY(config, EditAnywhere, Category = "Time")
	bool bPaused = false;

	/** Fallback social graph used when an NPC has none assigned. */
	UPROPERTY(config, EditAnywhere, Category = "Defaults")
	TSoftObjectPtr<USBF_SocialGraph> DefaultSocialGraph;

	/** Fallback behavior map used when an NPC has none assigned. */
	UPROPERTY(config, EditAnywhere, Category = "Defaults")
	TSoftObjectPtr<USBF_BehaviorMap> DefaultBehaviorMap;

	/** Fallback character trait used when an NPC has none assigned. */
	UPROPERTY(config, EditAnywhere, Category = "Defaults")
	TSoftObjectPtr<USBF_CharacterTrait> DefaultCharacterTrait;

	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }
	virtual FName GetSectionName() const override { return FName(TEXT("Social Behavior Framework")); }

	/** @return the singleton settings instance. */
	static const USBF_DeveloperSettings& Get();
};
