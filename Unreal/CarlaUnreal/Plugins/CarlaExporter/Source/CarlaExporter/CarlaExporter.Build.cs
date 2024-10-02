// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

using System;
using UnrealBuildTool;
using System.IO;

public class CarlaExporter :
	ModuleRules
{
	public CarlaExporter(ReadOnlyTargetRules Target) :
		base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    bEnableExceptions = true;



		PublicDependencyModuleNames.Add("Core");
		PublicDependencyModuleNames.Add("Carla");

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

    PublicDefinitions.AddRange(new string[]
    {
      "ASIO_NO_EXCEPTIONS",
      "BOOST_NO_EXCEPTIONS",
      "LIBCARLA_NO_EXCEPTIONS",
      "PUGIXML_NO_EXCEPTIONS",
      "BOOST_DISABLE_ABI_HEADERS",
      "BOOST_NO_RTTI",
      "BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY",
    });

    foreach (var Definition in File.ReadAllText(Path.Combine(PluginDirectory, "Definitions.def")).Split(';'))
      PrivateDefinitions.Add(Definition.Trim());

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Includes.def")).Split(';'))
      if (Path.Length != 0)
        PublicIncludePaths.Add(Path.Trim());

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Libraries.def")).Split(';'))
      if (Path.Length != 0)
        PublicAdditionalLibraries.Add(Path.Trim());
	}
}
