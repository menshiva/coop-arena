// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CoopArena : ModuleRules
{
	public CoopArena(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"UMG",
			"Slate"
		});
	}
}
