// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class USBF_CharacterTrait;
class USBF_TagLibrary;
class IDetailsView;
class SObjectPropertyEntryBox;
class SErrorText;
class SEditableTextBox;
class SSBF_CharacterTraitTab;

/** Fired when a tag row requests removal (parameter: tag string). */
DECLARE_DELEGATE_OneParam(FOnSBF_TagRemoveRequest, FString /*TagName*/);

/** Fired when a tag row commits a rename (old, new tag strings). */
DECLARE_DELEGATE_TwoParams(FOnSBF_TagRenameRequest, FString /*OldTagName*/, FString /*NewTagName*/);

/**
 * Single row of the tag library list: tag name with Remove / Rename actions.
 * Rename switches the row into inline editing and commits on Enter / focus
 * loss.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_TagRow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSBF_TagRow)
		: _TagName(TEXT(""))
		{}
		SLATE_ARGUMENT(FString, TagName)
		SLATE_EVENT(FOnSBF_TagRemoveRequest, OnRemove)
		SLATE_EVENT(FOnSBF_TagRenameRequest, OnRename)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnRemoveClicked();
	FReply OnRenameClicked();
	void CommitRename();
	FReply OnEditTextCommitted(const FText& NewText, ETextCommit::Type CommitType);

	FString TagName;
	bool bEditing = false;
	TSharedPtr<SEditableTextBox> EditBox;

	FOnSBF_TagRemoveRequest OnRemove;
	FOnSBF_TagRenameRequest OnRename;
};

/**
 * "Character Trait" tab of the unified editor window.
 *
 * Left: trait asset picker + details view of the selected USBF_CharacterTrait
 * (reaction table). Right: tag library asset picker + SListView tag editor
 * with Add / Remove / Rename and "Load Defaults" actions.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_CharacterTraitTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSBF_CharacterTraitTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Reloads the selected assets from disk (revert support). */
	void RefreshSelection();

	/** Validates the selected trait + library; shows errors inline. @return true when valid. */
	bool ValidateSelection();

	/** Sets the edited trait asset. */
	void SetTrait(USBF_CharacterTrait* NewTrait);

	/** Sets the edited tag library asset. */
	void SetTagLibrary(USBF_TagLibrary* NewLibrary);

	/** @return the currently edited trait. */
	USBF_CharacterTrait* GetTrait() const { return CurrentTrait; }

	/** @return the currently edited tag library. */
	USBF_TagLibrary* GetTagLibrary() const { return CurrentLibrary; }

private:
	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildLibraryEditor();

	FString GetTraitPath() const;
	FString GetLibraryPath() const;
	void OnTraitPicked(const FAssetData& AssetData);
	void OnLibraryPicked(const FAssetData& AssetData);

	FReply OnNewTraitClicked();
	FReply OnNewLibraryClicked();
	FReply OnSaveClicked();
	FReply OnRevertClicked();
	FReply OnValidateClicked();
	FReply OnAddTagClicked();
	FReply OnLoadDefaultsClicked();

	void RebuildTagList();
	void HandleTagRemove(FString TagName);
	void HandleTagRename(FString OldTagName, FString NewTagName);
	FText GetAddTagText() const;
	void OnAddTagTextChanged(const FText& NewText);

	USBF_CharacterTrait* CurrentTrait = nullptr;
	USBF_TagLibrary* CurrentLibrary = nullptr;
	FString CurrentTraitPath;
	FString CurrentLibraryPath;

	FString PendingTagName;

	TSharedPtr<SObjectPropertyEntryBox> TraitPicker;
	TSharedPtr<SObjectPropertyEntryBox> LibraryPicker;
	TSharedPtr<IDetailsView> TraitDetailsView;
	TSharedPtr<SErrorText> ValidationText;
	TSharedPtr<SListView<TSharedPtr<SSBF_TagRow>>> TagListView;
	TArray<TSharedPtr<SSBF_TagRow>> TagRows;
	TSharedPtr<SEditableTextBox> AddTagTextBox;
};
