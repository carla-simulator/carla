// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class CarlaTools : CarlaRules
{
  bool bUsingOmniverseConnector = false;

	public CarlaTools(ReadOnlyTargetRules Target) : base(Target, "../../../Carla/CarlaDependencies")
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    // PrivatePCHHeaderFile = "Carla.h";

    if (IsWindows())
    {
      bEnableExceptions = true;
    }

    string CarlaPluginPath = Path.GetFullPath( ModuleDirectory );
    string ConfigDir =  Path.GetFullPath(Path.Combine(CarlaPluginPath, "../../../../Config/"));
    string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
    string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);
    foreach (string line in text)
    {
      if (line.Contains("Omniverse ON"))
      {
        Console.WriteLine("Enabling OmniverseConnector");
        bUsingOmniverseConnector = true;
        PublicDefinitions.Add("WITH_OMNIVERSE");
        PrivateDefinitions.Add("WITH_OMNIVERSE");
      }

      if (line.Contains("Ros2 ON"))
      {
        Console.WriteLine("Enabling ros2");
        UsingRos2 = true;
      }

    }

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
        "Core",
        "ProceduralMeshComponent",
        "MeshDescription",
        "RawMesh",
        "AssetTools"
				// ... add other public dependencies that you statically link with here ...
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"Blutility",
				"UMG",
				"EditorScriptingUtilities",
				"Landscape",
				"Foliage",
				"FoliageEdit",
        "MeshMergeUtilities",
				"Carla",
        "StaticMeshDescription",
				"PhysXVehicles",
        "Json",
        "JsonUtilities",
        "Networking",
        "Sockets",
        "HTTP",
        "RHI",
        "RenderCore",
        "MeshMergeUtilities",
        "StreetMapImporting",
        "StreetMapRuntime"
				// ... add private dependencies that you statically link with here ...
			}
			);
    if(bUsingOmniverseConnector)
    {
      PrivateDependencyModuleNames.AddRange(
        new string[]
        {
          "OmniverseUSD",
          "OmniverseRuntime"
        });
    }

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		AddCarlaServerDependency();
	}

  
	private void AddCarlaServerDependency()
	{

    // Link dependencies.
    if (IsWindows())
    {
      AddBoostLibs();
    }
    AddStaticLibrary("rpc");
    if (UseDebugLibs())
    {
      AddStaticLibrary("carla_server_debug");
    }
    else
    {
      AddStaticLibrary("carla_server");
    }

    addOsmToODR();

    addROS2();

    // Include path.
    string LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");

    PublicIncludePaths.Add(LibCarlaIncludePath);
    PrivateIncludePaths.Add(LibCarlaIncludePath);

    PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
    PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
    PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
    PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
	}
}
