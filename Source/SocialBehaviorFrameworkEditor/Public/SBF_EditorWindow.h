// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetToolsModule.h"
#include "Factories/Factory.h"
#include "ObjectTools.h"
#include "UObject/Package.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class FTabManager;
class SSBF_SocialGraphTab;
class SSBF_BehaviorMapTab;
class SSBF_CharacterTraitTab;
class SDockTab;
class SWindow;

/**
 * Unified Social Behavior Framework editor window.
 *
 * Hosts a dock-tab layout with the three subsystem tabs (Social Graph,
 * Behavior Map, Character Trait) plus a common toolbar with
 * Refresh / Validate All / Preview in PIE actions and the world
 * visualization toggle.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API SSBF_EditorWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSBF_EditorWindow)
		: _OwnerTab()
		, _OwnerWindow()
		{}
		/** Dock tab hosting this window (needed to create the FTabManager). */
		SLATE_ARGUMENT(TSharedPtr<SDockTab>, OwnerTab)
		/** Window hosting the dock tab (passed to FTabManager::RestoreFrom). */
		SLATE_ARGUMENT(TSharedPtr<SWindow>, OwnerWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Shared asset helpers used by all three tabs. */
	template <typename T>
	static T* CreateAsset(const FString& AssetName, const FString& PackagePath, UClass* FactoryClass);

	static bool SaveAsset(UObject* Asset);

	/** Reloads an asset from disk (revert) and returns the reloaded instance. */
	static UObject* RevertAsset(UObject* Asset);

private:
	/** Tab spawners. */
	TSharedRef<SDockTab> SpawnSocialGraphTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnBehaviorMapTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnCharacterTraitTab(const FSpawnTabArgs& Args);

	/** Toolbar / action callbacks. */
	TSharedRef<SWidget> BuildToolbar();
	FReply OnRefreshClicked();
	FReply OnValidateAllClicked();
	FReply OnPreviewInPIEClicked();
	void OnDrawInViewportChanged(ECheckBoxState NewState);
	ECheckBoxState GetDrawInViewportState() const;
	FText GetStatusText() const;

	TSharedPtr<FTabManager> TabManager;
	TSharedPtr<SWindow> OwnerWindow;

	/** Persistent tab instances (state survives tab switching). */
	TSharedPtr<SSBF_SocialGraphTab> SocialGraphTab;
	TSharedPtr<SSBF_BehaviorMapTab> BehaviorMapTab;
	TSharedPtr<SSBF_CharacterTraitTab> CharacterTraitTab;

	FString StatusMessage;
};
