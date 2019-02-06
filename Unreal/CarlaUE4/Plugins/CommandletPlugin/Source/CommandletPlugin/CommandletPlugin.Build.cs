// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class CommandletPlugin : ModuleRules
	{
		public CommandletPlugin(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"Engine",
					"AssetRegistry",
					"UnrealEd",
					"AssetTools",
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"CommandletPlugin/Private",
					"CommandletPlugin/Private/Commandlets"
				});
		}
	}
}
