// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TickableEditorObject.h"
#include "UnrealEdGlobals.h"

/**
 * In-viewport relation visualizer (editor + PIE).
 *
 * Ticks with the editor and draws the social graph edges between the NPCs
 * present in the world (colored by resolved hostility: green = ally,
 * red = enemy, gray = neutral). Editor worlds use the world line batcher,
 * PIE worlds additionally use DrawDebugLine.
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API FSBF_Visualizer : public FTickableEditorObject
{
public:
	/** @return the process-wide visualizer singleton. */
	static FSBF_Visualizer& Get();

	/** Enables / disables drawing. */
	void SetEnabled(bool bInEnabled);

	/** @return true while drawing is enabled. */
	bool IsEnabled() const { return bEnabled; }

	// ~FTickableEditorObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bEnabled && GEditor != nullptr; }

private:
	FSBF_Visualizer() = default;

	/** Draws all relation edges of all NPCs present in the given world. */
	void DrawGraphEdges(UWorld* World, bool bIsPIE) const;

	bool bEnabled = true;
};
