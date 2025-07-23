// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class mg2 : ModuleRules
{
	public mg2(ReadOnlyTargetRules Target) : base(Target)
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
			"RenderCore",
			"Renderer",
			"RHI",
			"CinematicCamera",
			"ProceduralMeshComponent",
			"UMG"

		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Renderer",
			"RenderCore",
			"TerrainGen",
			"ProceduralMeshComponent",

		});
	}
}
