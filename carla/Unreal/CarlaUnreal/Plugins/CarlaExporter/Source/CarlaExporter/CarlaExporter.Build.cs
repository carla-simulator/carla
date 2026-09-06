// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

using UnrealBuildTool;

public class CarlaExporter :
	ModuleRules
{
	public CarlaExporter(ReadOnlyTargetRules Target) :
		base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.Add("Core");

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Projects",
			"InputCore",
			"UnrealEd",
			"LevelEditor",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"Chaos",
			"EditorStyle",
		});
	}
}
