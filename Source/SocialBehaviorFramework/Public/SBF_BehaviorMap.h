// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBF_Types.h"
#include "SBF_BehaviorMap.generated.h"

/**
 * Behavior Map data asset: where an NPC should be at any moment of the game
 * day. Work hours and (per weekday) leisure overrides are evaluated against
 * the time provided by USBF_TimeSubsystem; everything else resolves to Home.
 */
UCLASS(BlueprintType, hidecategories = (Object))
class SOCIALBEHAVIORFRAMEWORK_API USBF_BehaviorMap : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Location the NPC considers home. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FVector HomeLocation = FVector::ZeroVector;

	/** Location the NPC works at. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FVector WorkLocation = FVector::ZeroVector;

	/** Location the NPC spends leisure time at. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FVector LeisureLocation = FVector::ZeroVector;

	/** Daily work window (workdays only). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	FTimeRange WorkHours;

	/** Default leisure window used on days without a weekday override. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	FTimeRange DefaultLeisureHours;

	/**
	 * Per-weekday leisure overrides. Key is the weekday FName
	 * ("Monday" .. "Sunday").
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	TMap<FName, FTimeRange> LeisureByWeekday;

	/** Preferred leisure activity tags (gameplay hook for AI / BT). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule", meta = (Categories = "SBF.Leisure"))
	FGameplayTagContainer PreferredLeisureTags;

	/**
	 * Resolves the NPC activity for a given game time snapshot.
	 * @param Time current game time (from USBF_TimeSubsystem).
	 * @param OutActivity resolved activity.
	 * @param OutLocation resolved target location.
	 * @return false when the map is not configured (missing location data).
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool ResolveCurrentActivity(const FSBF_GameTime& Time, ESBF_ActivityType& OutActivity, FVector& OutLocation) const;

	/**
	 * Structural validation used by the editor window.
	 * @param OutErrors appended localized error descriptions.
	 * @return true when the map is usable.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool Validate(TArray<FText>& OutErrors) const;
};
