// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_EditorWindow.h"

#include "AssetToolsModule.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Docking/WorkspaceItem.h"
#include "Modules/ModuleManager.h"
#include "ObjectTools.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Factories/Factory.h"
#include "SBF_Log.h"
#include "SBF_Visualizer.h"
#include "Widgets/SBF_BehaviorMapTab.h"
#include "Widgets/SBF_CharacterTraitTab.h"
#include "Widgets/SBF_SocialGraphTab.h"

#define LOCTEXT_NAMESPACE "SSBF_EditorWindow"

void SSBF_EditorWindow::Construct(const FArguments& InArgs)
{
	// Persistent tab widgets: keep state while the window is alive.
	SAssignNew(SocialGraphTab, SSBF_SocialGraphTab);
	SAssignNew(BehaviorMapTab, SSBF_BehaviorMapTab);
	SAssignNew(CharacterTraitTab, SSBF_CharacterTraitTab);

	checkf(InArgs._OwnerTab.IsValid(), TEXT("SSBF_EditorWindow must be spawned inside an SDockTab (see FSBF_EditorModule::SpawnMainTab)."));
	TabManager = FGlobalTabmanager::Get()->NewTabManager(InArgs._OwnerTab.ToSharedRef());
	OwnerWindow = InArgs._OwnerWindow;

	TabManager->RegisterTabSpawner(FName(TEXT("SBF_SocialGraphTab")), FOnSpawnTab::CreateRaw(this, &SSBF_EditorWindow::SpawnSocialGraphTab))
		.SetDisplayName(LOCTEXT("SocialGraphTab", "Social Graph"))
		.SetTooltipText(LOCTEXT("SocialGraphTabTooltip", "Edit the NPC relationship hierarchy and explicit relation edges."));

	TabManager->RegisterTabSpawner(FName(TEXT("SBF_BehaviorMapTab")), FOnSpawnTab::CreateRaw(this, &SSBF_EditorWindow::SpawnBehaviorMapTab))
		.SetDisplayName(LOCTEXT("BehaviorMapTab", "Behavior Map"))
		.SetTooltipText(LOCTEXT("BehaviorMapTabTooltip", "Edit the NPC daily schedule (Home / Work / Leisure) with a timeline preview."));

	TabManager->RegisterTabSpawner(FName(TEXT("SBF_CharacterTraitTab")), FOnSpawnTab::CreateRaw(this, &SSBF_EditorWindow::SpawnCharacterTraitTab))
		.SetDisplayName(LOCTEXT("CharacterTraitTab", "Character Trait"))
		.SetTooltipText(LOCTEXT("CharacterTraitTabTooltip", "Edit character traits, reactions and the shared tag library."));

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(TEXT("SBF_EditorWindow_Layout_v1"))
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.45f)
				->AddTab(FName(TEXT("SBF_SocialGraphTab")), ETabState::OpenedTab)
				->SetForegroundTab(FName(TEXT("SBF_SocialGraphTab")))
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.30f)
				->AddTab(FName(TEXT("SBF_BehaviorMapTab")), ETabState::OpenedTab)
				->AddTab(FName(TEXT("SBF_CharacterTraitTab")), ETabState::OpenedTab)
			)
		);

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
		.FillHeight(1.0f)
		[
			TabManager->RestoreFrom(Layout, OwnerWindow).ToSharedRef()
		]
	];
}

TSharedRef<SDockTab> SSBF_EditorWindow::SpawnSocialGraphTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(LOCTEXT("SocialGraphTab", "Social Graph"))
		[
			SocialGraphTab.ToSharedRef()
		];
}

TSharedRef<SDockTab> SSBF_EditorWindow::SpawnBehaviorMapTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(LOCTEXT("BehaviorMapTab", "Behavior Map"))
		[
			BehaviorMapTab.ToSharedRef()
		];
}

TSharedRef<SDockTab> SSBF_EditorWindow::SpawnCharacterTraitTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(LOCTEXT("CharacterTraitTab", "Character Trait"))
		[
			CharacterTraitTab.ToSharedRef()
		];
}

TSharedRef<SWidget> SSBF_EditorWindow::BuildToolbar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
			.ToolTipText(LOCTEXT("RefreshTooltip", "Reloads the currently selected assets from disk."))
			.OnClicked(this, &SSBF_EditorWindow::OnRefreshClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ValidateAll", "Validate"))
			.ToolTipText(LOCTEXT("ValidateAllTooltip", "Validates the assets selected in all three tabs."))
			.OnClicked(this, &SSBF_EditorWindow::OnValidateAllClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("PreviewInPIE", "Preview in PIE"))
			.ToolTipText(LOCTEXT("PreviewInPIETooltip", "Launches PIE at the default player start to preview NPC schedules and relations."))
			.OnClicked(this, &SSBF_EditorWindow::OnPreviewInPIEClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SSBF_EditorWindow::GetDrawInViewportState)
			.OnCheckStateChanged(this, &SSBF_EditorWindow::OnDrawInViewportChanged)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DrawInViewport", "Draw in Viewport"))
				.ToolTipText(LOCTEXT("DrawInViewportTooltip", "Toggles the colored relation edges in the editor viewport and in PIE."))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8.0f, 4.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &SSBF_EditorWindow::GetStatusText)
			.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
		];
}

FReply SSBF_EditorWindow::OnRefreshClicked()
{
	SocialGraphTab->RefreshSelection();
	BehaviorMapTab->RefreshSelection();
	CharacterTraitTab->RefreshSelection();
	StatusMessage = TEXT("Assets refreshed.");
	return FReply::Handled();
}

FReply SSBF_EditorWindow::OnValidateAllClicked()
{
	const bool bGraphValid = SocialGraphTab->ValidateSelection();
	const bool bMapValid = BehaviorMapTab->ValidateSelection();
	const bool bTraitValid = CharacterTraitTab->ValidateSelection();
	StatusMessage = (bGraphValid && bMapValid && bTraitValid) ? TEXT("Validation passed.") : TEXT("Validation failed - see tab errors and the output log.");
	return FReply::Handled();
}

FReply SSBF_EditorWindow::OnPreviewInPIEClicked()
{
	if (GEditor)
	{
		GEditor->RequestPlaySession(/*bAtPlayerStart=*/false);
		StatusMessage = TEXT("PIE requested.");
	}
	return FReply::Handled();
}

void SSBF_EditorWindow::OnDrawInViewportChanged(ECheckBoxState NewState)
{
	FSBF_Visualizer::Get().SetEnabled(NewState == ECheckBoxState::Checked);
}

ECheckBoxState SSBF_EditorWindow::GetDrawInViewportState() const
{
	return FSBF_Visualizer::Get().IsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FText SSBF_EditorWindow::GetStatusText() const
{
	return FText::FromString(StatusMessage);
}

template <typename T>
T* SSBF_EditorWindow::CreateAsset(const FString& AssetName, const FString& PackagePath, UClass* FactoryClass)
{
	UFactory* Factory = NewObject<UFactory>(GetTransientPackage(), FactoryClass);
	const FString SanitizedName = ObjectTools::SanitizeObjectName(AssetName);
	UObject* Created = FAssetToolsModule::GetModule().Get().CreateAsset(SanitizedName, PackagePath, T::StaticClass(), Factory);
	return Cast<T>(Created);
}

bool SSBF_EditorWindow::SaveAsset(UObject* Asset)
{
	if (!Asset)
	{
		return false;
	}

	Asset->MarkPackageDirty();
	TArray<UPackage*> Packages;
	Packages.Add(Asset->GetOutermost());
	return FEditorFileUtils::PromptForCheckoutAndSave(Packages, /*bCheckDirty=*/true, /*bPromptToSave=*/false);
}

UObject* SSBF_EditorWindow::RevertAsset(UObject* Asset)
{
	if (!Asset)
	{
		return nullptr;
	}

	UPackage* Package = Asset->GetOutermost();
	const FString Path = Asset->GetPathName();

	ResetLoaders(Package);
	Package->FullyLoad();

	return FindObject<UObject>(nullptr, *Path);
}

#undef LOCTEXT_NAMESPACE
