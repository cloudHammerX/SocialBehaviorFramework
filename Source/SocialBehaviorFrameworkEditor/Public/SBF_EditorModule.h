// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FSpawnTabArgs;
class SDockTab;

/**
 * Editor module of the Social Behavior Framework.
 *
 * Registers:
 *  - the "Social Behavior Framework" nomad tab (Window menu -> ToolMenus),
 *  - the details customizations for USBF_SocialRelation / USBF_NPCComponent,
 *  - the in-viewport relation visualizer (FSBF_Visualizer).
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API FSBF_EditorModule : public IModuleInterface
{
public:
	/** Tab identifier of the unified editor window. */
	static const FName MainTabId;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Opens (or focuses) the unified editor window. */
	void OpenSBFWindow();

private:
	/** Registers the Window-menu entry through ToolMenus. */
	void RegisterMenuExtensions();

	/** Spawns the unified editor window dock tab. */
	TSharedRef<SDockTab> SpawnMainTab(const FSpawnTabArgs& Args);
};
