// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_EditorModule.h"

#include "Framework/Docking/TabManager.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "SBF_NPCComponent.h"
#include "SBF_SocialRelation.h"
#include "SBF_EditorWindow.h"
#include "SBF_Visualizer.h"
#include "Customizations/SBF_NPCComponentCustomization.h"
#include "Customizations/SBF_SocialRelationCustomization.h"

#define LOCTEXT_NAMESPACE "FSBF_EditorModule"

const FName FSBF_EditorModule::MainTabId = FName(TEXT("SBF_MainTab"));

void FSBF_EditorModule::StartupModule()
{
	// Ensure the world visualizer singleton exists and starts ticking.
	FSBF_Visualizer::Get();

	// Unified editor window (nomad tab).
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		MainTabId,
		FOnSpawnTab::CreateRaw(this, &FSBF_EditorModule::SpawnMainTab))
		.SetDisplayName(LOCTEXT("SBFWindowTitle", "Social Behavior Framework"))
		.SetTooltipText(LOCTEXT("SBFWindowTooltip", "Social Graph, Behavior Map and Character Trait editing, visualization and validation."))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// Window menu entry (Window -> Social Behavior Framework).
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSBF_EditorModule::RegisterMenuExtensions));

	// Details customizations.
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		USBF_SocialRelation::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSBF_SocialRelationCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		USBF_NPCComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSBF_NPCComponentCustomization::MakeInstance));
}

void FSBF_EditorModule::ShutdownModule()
{
	UToolMenus::UnregisterStartupCallback(this);
	UToolMenus::UnRegisterOwner("SocialBehaviorFramework");

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MainTabId);

	if (FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyModule->UnregisterCustomClassLayout(USBF_SocialRelation::StaticClass()->GetFName());
		PropertyModule->UnregisterCustomClassLayout(USBF_NPCComponent::StaticClass()->GetFName());
	}
}

void FSBF_EditorModule::OpenSBFWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MainTabId);
}

void FSBF_EditorModule::RegisterMenuExtensions()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Window");
	FToolMenuSection& Section = WindowMenu->FindOrAddSection("WindowLayout");
	Section.AddMenuEntry(
		FName(TEXT("SocialBehaviorFrameworkWindow")),
		LOCTEXT("SBFWindowMenuLabel", "Social Behavior Framework"),
		LOCTEXT("SBFWindowMenuTooltip", "Open the Social Behavior Framework editor window."),
		FSlateIcon(),
		FToolUIActionChoice(FExecuteAction::CreateRaw(this, &FSBF_EditorModule::OpenSBFWindow)));
}

TSharedRef<SDockTab> FSBF_EditorModule::SpawnMainTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> Tab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("SBFWindowTitle", "Social Behavior Framework"));

	Tab->SetContent(SNew(SSBF_EditorWindow).OwnerTab(Tab).OwnerWindow(Args.GetOwnerWindow()));
	return Tab;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSBF_EditorModule, SocialBehaviorFrameworkEditor)
