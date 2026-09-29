// Copyright Social Behavior Framework. All Rights Reserved.

using UnrealBuildTool;

/**
 * Build rules for the SocialBehaviorFrameworkTests module.
 *
 * Automation (FAutomationTestBase) only. The automation framework lives in
 * Core, so this module has no editor dependencies and can run through the
 * Session Frontend, `Automation RunTests` console command or the command line.
 */
public class SocialBehaviorFrameworkTests : ModuleRules
{
	public SocialBehaviorFrameworkTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AIModule",
			"GameplayTags",
			"SocialBehaviorFramework"
		});
	}
}
