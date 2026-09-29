// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_Types.h"

FSBF_GameTime FSBF_GameTime::FromTotalMinutes(int64 TotalMinutes, const TArray<FName>& WeekendDays)
{
	static const FName WeekdayNames[7] =
	{
		FName(TEXT("Monday")), FName(TEXT("Tuesday")), FName(TEXT("Wednesday")),
		FName(TEXT("Thursday")), FName(TEXT("Friday")), FName(TEXT("Saturday")), FName(TEXT("Sunday"))
	};

	const int64 DayIndex = TotalMinutes / (24 * 60);
	const int32 MinutesIntoDay = static_cast<int32>(TotalMinutes % (24 * 60));

	FSBF_GameTime Time;
	Time.Hour = MinutesIntoDay / 60;
	Time.Minute = MinutesIntoDay % 60;
	Time.Day = static_cast<int32>(DayIndex) + 1;
	Time.Weekday = WeekdayNames[DayIndex % 7];
	Time.bIsWeekend = WeekendDays.Contains(Time.Weekday);
	return Time;
}
