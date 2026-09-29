// Copyright Social Behavior Framework. All Rights Reserved.

using UnrealBuildTool;

/**
 * Build rules for the SocialBehaviorFramework runtime module.
 *
 * The module is C++20, desktop oriented (Win64 / Linux / Mac) and depends only
 * on engine runtime modules. UMG is required for the optional per-NPC relation
 * widget (UUserWidget / UWidgetComponent); Slate/SlateCore are pulled in for
 * shared widget types referenced from data assets.
 */
public class SocialBehaviorFramework : ModuleRules
{
	public SocialBehaviorFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AIModule",
			"GameplayTasks",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore",
			"DeveloperSettings"
		});
	}
}
