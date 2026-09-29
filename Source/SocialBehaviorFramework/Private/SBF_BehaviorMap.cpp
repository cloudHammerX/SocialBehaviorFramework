// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_BehaviorMap.h"

#include "SBF_Log.h"

bool USBF_BehaviorMap::ResolveCurrentActivity(const FSBF_GameTime& Time, ESBF_ActivityType& OutActivity, FVector& OutLocation) const
{
	OutActivity = ESBF_ActivityType::None;
	OutLocation = FVector::ZeroVector;

	const int32 MinuteOfDay = Time.TotalMinuteOfDay();

	// Workdays (and when weekend support is disabled) honor WorkHours.
	if (!Time.bIsWeekend && WorkHours.Contains(MinuteOfDay))
	{
		OutActivity = ESBF_ActivityType::Work;
		OutLocation = WorkLocation;
		return true;
	}

	// Leisure: weekday override wins over the default window.
	FTimeRange Leisure = DefaultLeisureHours;
	if (const FTimeRange* Override = LeisureByWeekday.Find(Time.Weekday))
	{
		Leisure = *Override;
	}

	if (Leisure.Contains(MinuteOfDay))
	{
		OutActivity = ESBF_ActivityType::Leisure;
		OutLocation = LeisureLocation;
		return true;
	}

	OutActivity = ESBF_ActivityType::Home;
	OutLocation = HomeLocation;
	return true;
}

bool USBF_BehaviorMap::Validate(TArray<FText>& OutErrors) const
{
	bool bValid = true;

	if (HomeLocation.IsZero())
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_MapNoHome", "Behavior Map: HomeLocation is not set."));
		bValid = false;
	}
	if (WorkLocation.IsZero())
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_MapNoWork", "Behavior Map: WorkLocation is not set."));
		bValid = false;
	}
	if (LeisureLocation.IsZero())
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_MapNoLeisure", "Behavior Map: LeisureLocation is not set."));
		bValid = false;
	}
	if (!WorkHours.IsValid())
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_MapNoWorkHours", "Behavior Map: WorkHours range is empty (start == end)."));
		bValid = false;
	}
	if (!DefaultLeisureHours.IsValid())
	{
		OutErrors.Add(NSLOCTEXT("SBF", "SBF_MapNoLeisureHours", "Behavior Map: DefaultLeisureHours range is empty (start == end)."));
		bValid = false;
	}

	for (const TPair<FName, FTimeRange>& Pair : LeisureByWeekday)
	{
		if (!Pair.Value.IsValid())
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("Behavior Map: leisure override for '%s' is an empty range."), *Pair.Key.ToString())));
			bValid = false;
		}
	}

	if (!bValid)
	{
		UE_LOG(LogSBF, Warning, TEXT("BehaviorMap '%s' failed validation (%d errors)."), *GetName(), OutErrors.Num());
	}
	return bValid;
}
