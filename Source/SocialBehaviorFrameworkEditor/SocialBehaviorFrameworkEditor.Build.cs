// Copyright Social Behavior Framework. All Rights Reserved.

using UnrealBuildTool;

/**
 * Build rules for the SocialBehaviorFrameworkEditor module (UncookedOnly).
 *
 * Hosts the unified editor window, the in-viewport relation visualizer,
 * details customizations and the UFactory classes used to create SBF data
 * assets from the Content Browser and from the SBF editor window.
 */
public class SocialBehaviorFrameworkEditor : ModuleRules
{
	public SocialBehaviorFrameworkEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"Slate",
			"SlateCore",
			"InputCore",
			"AIModule",
			"GameplayTags",
			"SocialBehaviorFramework",
			"PropertyEditor",
			"GraphEditor",
			"AssetTools",
			"ToolMenus",
			"Kismet",
			"KismetCompiler",
			"EditorStyle",
			"MessageLog"
		});
	}
}
