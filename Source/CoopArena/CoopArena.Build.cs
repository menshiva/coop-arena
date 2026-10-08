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
			"Slate",
			"SlateCore",
			"GameplayTags",
			"GameplayTasks",
			"GameplayAbilities",
			"NavigationSystem",
			"RHI"
		});

		PublicIncludePaths.Add(ModuleDirectory);

		// the bench reads the process's video memory from DXGI
		PublicSystemLibraries.Add("dxgi.lib");
	}
}
