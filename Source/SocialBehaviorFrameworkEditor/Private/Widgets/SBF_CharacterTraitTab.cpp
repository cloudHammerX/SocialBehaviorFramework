// Copyright Social Behavior Framework. All Rights Reserved.

#include "Widgets/SBF_CharacterTraitTab.h"

#include "Framework/Application/SlateApplication.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SObjectPropertyEntryBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/SErrorText.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "IDetailsView.h"
#include "SBF_AssetFactories.h"
#include "SBF_CharacterTrait.h"
#include "SBF_DeveloperSettings.h"
#include "SBF_EditorWindow.h"
#include "SBF_Log.h"
#include "SBF_TagLibrary.h"

#define LOCTEXT_NAMESPACE "SSBF_CharacterTraitTab"

// =============================================================================
// SSBF_TagRow
// =============================================================================

void SSBF_TagRow::Construct(const FArguments& InArgs)
{
	TagName = InArgs._TagName;
	OnRemove = InArgs._OnRemove;
	OnRename = InArgs._OnRename;

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TagName))
			.Visibility_Lambda([this]() { return bEditing ? EVisibility::Collapsed : EVisibility::Visible; })
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(EditBox, SEditableTextBox)
			.Text(FText::FromString(TagName))
			.Visibility_Lambda([this]() { return bEditing ? EVisibility::Visible : EVisibility::Collapsed; })
			.OnTextCommitted(this, &SSBF_TagRow::OnEditTextCommitted)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Rename", "Rename"))
			.OnClicked(this, &SSBF_TagRow::OnRenameClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Remove", "Remove"))
			.OnClicked(this, &SSBF_TagRow::OnRemoveClicked)
		]
	];
}

FReply SSBF_TagRow::OnRemoveClicked()
{
	OnRemove.ExecuteIfBound(TagName);
	return FReply::Handled();
}

FReply SSBF_TagRow::OnRenameClicked()
{
	bEditing = true;
	if (EditBox)
	{
		EditBox->SetText(FText::FromString(TagName));
		FSlateApplication::Get().SetUserFocus(0, EditBox.ToSharedRef(), EFocusCause::SetDirectly);
	}
	Invalidate(EInvalidateWidget::LayoutAndVolatility);
	return FReply::Handled();
}

void SSBF_TagRow::CommitRename()
{
	if (!bEditing)
	{
		return;
	}
	bEditing = false;
	if (EditBox)
	{
		const FString NewName = EditBox->GetText().ToString();
		if (!NewName.IsEmpty() && NewName != TagName)
		{
			OnRename.ExecuteIfBound(TagName, NewName);
		}
	}
	Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

FReply SSBF_TagRow::OnEditTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	CommitRename();
	return FReply::Handled();
}

// =============================================================================
// SSBF_CharacterTraitTab
// =============================================================================

void SSBF_CharacterTraitTab::Construct(const FArguments& InArgs)
{
	const USBF_CharacterTrait* DefaultTrait = USBF_DeveloperSettings::Get().DefaultCharacterTrait.LoadSynchronous();
	CurrentTrait = const_cast<USBF_CharacterTrait*>(DefaultTrait);
	if (CurrentTrait)
	{
		CurrentTraitPath = CurrentTrait->GetPathName();
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bUpdatesFromSelection = false;
	DetailsArgs.bLockable = false;
	DetailsArgs.bAllowSearch = true;
	DetailsArgs.bShowOptions = true;
	TraitDetailsView = PropertyModule.CreateDetailView(DetailsArgs);
	if (CurrentTrait)
	{
		TraitDetailsView->SetObject(CurrentTrait);
	}

	SAssignNew(ValidationText, SErrorText);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			BuildToolbar()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 0.0f)
		[
			ValidationText.ToSharedRef()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.55f)
			.Padding(4.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					TraitDetailsView.ToSharedRef()
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.45f)
			.Padding(4.0f)
			[
				BuildLibraryEditor()
			]
		]
	];

	RebuildTagList();
}

TSharedRef<SWidget> SSBF_CharacterTraitTab::BuildToolbar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TraitLabel", "Trait:"))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SAssignNew(TraitPicker, SObjectPropertyEntryBox)
			.AllowedClass(USBF_CharacterTrait::StaticClass())
			.AllowClear(true)
			.ObjectPath_Lambda([this]() { return GetTraitPath(); })
			.OnObjectChanged(this, &SSBF_CharacterTraitTab::OnTraitPicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("LibraryLabel", "Library:"))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SAssignNew(LibraryPicker, SObjectPropertyEntryBox)
			.AllowedClass(USBF_TagLibrary::StaticClass())
			.AllowClear(true)
			.ObjectPath_Lambda([this]() { return GetLibraryPath(); })
			.OnObjectChanged(this, &SSBF_CharacterTraitTab::OnLibraryPicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("NewTrait", "New Trait"))
			.OnClicked(this, &SSBF_CharacterTraitTab::OnNewTraitClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("NewLibrary", "New Library"))
			.OnClicked(this, &SSBF_CharacterTraitTab::OnNewLibraryClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Save", "Save"))
			.OnClicked(this, &SSBF_CharacterTraitTab::OnSaveClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Revert", "Revert"))
			.OnClicked(this, &SSBF_CharacterTraitTab::OnRevertClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Validate", "Validate"))
			.OnClicked(this, &SSBF_CharacterTraitTab::OnValidateClicked)
		];
}

TSharedRef<SWidget> SSBF_CharacterTraitTab::BuildLibraryEditor()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(6.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TagLibrarySection", "Tag Library"))
				.Font(FAppStyle::GetFontStyle("NormalFontBold"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SAssignNew(AddTagTextBox, SEditableTextBox)
					.HintText(LOCTEXT("AddTagHint", "Threat.Life"))
					.OnTextChanged(this, &SSBF_CharacterTraitTab::OnAddTagTextChanged)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("AddTag", "Add"))
					.OnClicked(this, &SSBF_CharacterTraitTab::OnAddTagClicked)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("LoadDefaults", "Load Defaults"))
					.ToolTipText(LOCTEXT("LoadDefaultsTooltip", "Replaces the set with the framework default trigger tags."))
					.OnClicked(this, &SSBF_CharacterTraitTab::OnLoadDefaultsClicked)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(TagListView, SListView<TSharedPtr<SSBF_TagRow>>)
					.ListItemsSource(&TagRows)
					.OnGenerateRow_Lambda([](TSharedPtr<SSBF_TagRow> Row, const TSharedRef<STableViewBase>& Owner)
					{
						return SNew(STableRow<TSharedPtr<SSBF_TagRow>>, Owner)
						[
							Row.ToSharedRef()
						];
					})
				]
			]
		];
}

FString SSBF_CharacterTraitTab::GetTraitPath() const
{
	return CurrentTraitPath;
}

FString SSBF_CharacterTraitTab::GetLibraryPath() const
{
	return CurrentLibraryPath;
}

void SSBF_CharacterTraitTab::OnTraitPicked(const FAssetData& AssetData)
{
	UObject* Asset = AssetData.GetAsset();
	if (!Asset)
	{
		SetTrait(nullptr);
		return;
	}

	USBF_CharacterTrait* Trait = Cast<USBF_CharacterTrait>(Asset);
	if (!Trait)
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: selected asset is not a USBF_CharacterTrait."));
		return;
	}
	SetTrait(Trait);
}

void SSBF_CharacterTraitTab::OnLibraryPicked(const FAssetData& AssetData)
{
	UObject* Asset = AssetData.GetAsset();
	if (!Asset)
	{
		SetTagLibrary(nullptr);
		return;
	}

	USBF_TagLibrary* Library = Cast<USBF_TagLibrary>(Asset);
	if (!Library)
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: selected asset is not a USBF_TagLibrary."));
		return;
	}
	SetTagLibrary(Library);
}

void SSBF_CharacterTraitTab::SetTrait(USBF_CharacterTrait* NewTrait)
{
	CurrentTrait = NewTrait;
	CurrentTraitPath = NewTrait ? NewTrait->GetPathName() : FString();

	if (TraitDetailsView)
	{
		TraitDetailsView->SetObject(NewTrait);
	}
}

void SSBF_CharacterTraitTab::SetTagLibrary(USBF_TagLibrary* NewLibrary)
{
	CurrentLibrary = NewLibrary;
	CurrentLibraryPath = NewLibrary ? NewLibrary->GetPathName() : FString();
	RebuildTagList();
}

void SSBF_CharacterTraitTab::RefreshSelection()
{
	if (!CurrentTraitPath.IsEmpty())
	{
		SetTrait(LoadObject<USBF_CharacterTrait>(nullptr, *CurrentTraitPath));
	}
	if (!CurrentLibraryPath.IsEmpty())
	{
		SetTagLibrary(LoadObject<USBF_TagLibrary>(nullptr, *CurrentLibraryPath));
	}
}

bool SSBF_CharacterTraitTab::ValidateSelection()
{
	if (!CurrentTrait)
	{
		ValidationText->SetError(LOCTEXT("NoTraitSelected", "No Character Trait selected."));
		return false;
	}

	TArray<FText> Errors;
	const bool bTraitValid = CurrentTrait->Validate(Errors);

	bool bLibraryValid = true;
	if (CurrentLibrary)
	{
		bLibraryValid = CurrentLibrary->Validate(Errors);
	}
	else
	{
		Errors.Add(LOCTEXT("NoLibrarySelected", "No Tag Library selected."));
		bLibraryValid = false;
	}

	if (bTraitValid && bLibraryValid)
	{
		ValidationText->SetError(FText::GetEmpty());
		return true;
	}

	FString Joined;
	for (const FText& Error : Errors)
	{
		Joined += Error.ToString();
		Joined += TEXT("\n");
		UE_LOG(LogSBF, Warning, TEXT("SBF validation: %s"), *Error.ToString());
	}
	ValidationText->SetError(FText::FromString(Joined));
	return false;
}

FReply SSBF_CharacterTraitTab::OnNewTraitClicked()
{
	USBF_CharacterTrait* NewTrait = SSBF_EditorWindow::CreateAsset<USBF_CharacterTrait>(
		TEXT("SBF_Trait"), TEXT("/Game/SBF"), USBF_CharacterTraitFactory::StaticClass());
	if (NewTrait)
	{
		SetTrait(NewTrait);
	}
	else
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: failed to create a new Character Trait."));
	}
	return FReply::Handled();
}

FReply SSBF_CharacterTraitTab::OnNewLibraryClicked()
{
	USBF_TagLibrary* NewLibrary = SSBF_EditorWindow::CreateAsset<USBF_TagLibrary>(
		TEXT("SBF_TagLibrary"), TEXT("/Game/SBF"), USBF_TagLibraryFactory::StaticClass());
	if (NewLibrary)
	{
		SetTagLibrary(NewLibrary);
	}
	else
	{
		UE_LOG(LogSBF, Warning, TEXT("SBF editor: failed to create a new Tag Library."));
	}
	return FReply::Handled();
}

FReply SSBF_CharacterTraitTab::OnSaveClicked()
{
	SSBF_EditorWindow::SaveAsset(CurrentTrait);
	SSBF_EditorWindow::SaveAsset(CurrentLibrary);
	return FReply::Handled();
}

FReply SSBF_CharacterTraitTab::OnRevertClicked()
{
	if (CurrentTrait)
	{
		SetTrait(Cast<USBF_CharacterTrait>(SSBF_EditorWindow::RevertAsset(CurrentTrait)));
	}
	if (CurrentLibrary)
	{
		SetTagLibrary(Cast<USBF_TagLibrary>(SSBF_EditorWindow::RevertAsset(CurrentLibrary)));
	}
	return FReply::Handled();
}

FReply SSBF_CharacterTraitTab::OnValidateClicked()
{
	ValidateSelection();
	return FReply::Handled();
}

FReply SSBF_CharacterTraitTab::OnAddTagClicked()
{
	if (!CurrentLibrary)
	{
		ValidationText->SetError(LOCTEXT("NoLibraryForTag", "Select or create a Tag Library first."));
		return FReply::Handled();
	}

	const FString TagName = PendingTagName.TrimStartAndEnd();
	if (TagName.IsEmpty())
	{
		return FReply::Handled();
	}

	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagName), /*bErrorIfNotFound=*/false);
	if (!Tag.IsValid())
	{
		ValidationText->SetError(FText::FromString(FString::Printf(
			TEXT("Tag '%s' does not exist. Add it to DefaultGameplayTags.ini first (or use Load Defaults)."), *TagName)));
		return FReply::Handled();
	}

	CurrentLibrary->AddTag(Tag);
	PendingTagName.Empty();
	if (AddTagTextBox)
	{
		AddTagTextBox->SetText(FText::GetEmpty());
	}
	ValidationText->SetError(FText::GetEmpty());
	RebuildTagList();
	return FReply::Handled();
}

FReply SSBF_CharacterTraitTab::OnLoadDefaultsClicked()
{
	if (!CurrentLibrary)
	{
		ValidationText->SetError(LOCTEXT("NoLibraryForDefaults", "Select or create a Tag Library first."));
		return FReply::Handled();
	}

	CurrentLibrary->LoadDefaultTags();
	RebuildTagList();
	return FReply::Handled();
}

void SSBF_CharacterTraitTab::RebuildTagList()
{
	TagRows.Empty();
	if (CurrentLibrary)
	{
		for (const FGameplayTag& Tag : CurrentLibrary->TriggerTags)
		{
			const FString Name = Tag.ToString();
			TagRows.Add(SNew(SSBF_TagRow)
				.TagName(Name)
				.OnRemove(this, &SSBF_CharacterTraitTab::HandleTagRemove)
				.OnRename(this, &SSBF_CharacterTraitTab::HandleTagRename));
		}
	}
	if (TagListView)
	{
		TagListView->RequestListRefresh();
	}
}

void SSBF_CharacterTraitTab::HandleTagRemove(FString TagName)
{
	if (!CurrentLibrary)
	{
		return;
	}

	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagName), /*bErrorIfNotFound=*/false);
	if (Tag.IsValid())
	{
		CurrentLibrary->RemoveTag(Tag);
	}
	RebuildTagList();
}

void SSBF_CharacterTraitTab::HandleTagRename(FString OldTagName, FString NewTagName)
{
	if (!CurrentLibrary)
	{
		return;
	}

	const FGameplayTag OldTag = FGameplayTag::RequestGameplayTag(FName(*OldTagName), /*bErrorIfNotFound=*/false);
	const FGameplayTag NewTag = FGameplayTag::RequestGameplayTag(FName(*NewTagName), /*bErrorIfNotFound=*/false);
	if (!NewTag.IsValid())
	{
		ValidationText->SetError(FText::FromString(FString::Printf(
			TEXT("Tag '%s' does not exist. Add it to DefaultGameplayTags.ini first."), *NewTagName)));
		return;
	}

	if (OldTag.IsValid())
	{
		CurrentLibrary->RenameTag(OldTag, NewTag);
	}
	else
	{
		// Old tag not resolvable anymore (e.g. freshly typed) - add the new one.
		CurrentLibrary->AddTag(NewTag);
	}
	ValidationText->SetError(FText::GetEmpty());
	RebuildTagList();
}

FText SSBF_CharacterTraitTab::GetAddTagText() const
{
	return FText::FromString(PendingTagName);
}

void SSBF_CharacterTraitTab::OnAddTagTextChanged(const FText& NewText)
{
	PendingTagName = NewText.ToString();
}

#undef LOCTEXT_NAMESPACE
