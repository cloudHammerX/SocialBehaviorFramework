// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBF_TagLibrary.generated.h"

/**
 * Tag Library data asset: the editable set of trigger tags shared by the
 * project ("Threat.Life", "Work.Late", "Offender.Detected", ...).
 *
 * Tags referenced by reactions should exist in this library (or in the
 * project's DefaultGameplayTags.ini). The editor window ships a
 * "Load Defaults" action that restores the framework's default set.
 */
UCLASS(BlueprintType, hidecategories = (Object))
class SOCIALBEHAVIORFRAMEWORK_API USBF_TagLibrary : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Editable trigger tag set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags", meta = (Categories = "SBF"))
	TArray<FGameplayTag> TriggerTags;

	/** @return true when the library already contains the tag. */
	UFUNCTION(BlueprintPure, Category = "SBF")
	bool ContainsTag(FGameplayTag Tag) const;

	/**
	 * Adds a tag.
	 * @return true when the tag was added (false on duplicate / invalid tag).
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool AddTag(FGameplayTag Tag);

	/** Removes a tag by exact match. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool RemoveTag(FGameplayTag Tag);

	/**
	 * Renames a tag (replaces OldTag with NewTag).
	 * @return true when a change was applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool RenameTag(FGameplayTag OldTag, FGameplayTag NewTag);

	/** Replaces the whole set with the framework's default trigger tags. */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	void LoadDefaultTags();

	/**
	 * Structural validation used by the editor window.
	 * @param OutErrors appended localized error descriptions.
	 * @return true when the library is usable.
	 */
	UFUNCTION(BlueprintCallable, Category = "SBF")
	bool Validate(TArray<FText>& OutErrors) const;

	/** @return the default trigger tags defined by the framework. */
	static TArray<FGameplayTag> GetDefaultTriggerTags();
};
