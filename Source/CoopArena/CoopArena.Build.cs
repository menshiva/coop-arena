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
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CoopArena",
			"CoopArena/Variant_Platforming",
			"CoopArena/Variant_Platforming/Animation",
			"CoopArena/Variant_Combat",
			"CoopArena/Variant_Combat/AI",
			"CoopArena/Variant_Combat/Animation",
			"CoopArena/Variant_Combat/Gameplay",
			"CoopArena/Variant_Combat/Interfaces",
			"CoopArena/Variant_Combat/UI",
			"CoopArena/Variant_SideScrolling",
			"CoopArena/Variant_SideScrolling/AI",
			"CoopArena/Variant_SideScrolling/Gameplay",
			"CoopArena/Variant_SideScrolling/Interfaces",
			"CoopArena/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
