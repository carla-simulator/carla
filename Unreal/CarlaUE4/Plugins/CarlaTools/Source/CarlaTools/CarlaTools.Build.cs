// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class CarlaTools : ModuleRules
{
  bool UsingHoudini = true;
  bool bUsingOmniverseConnector = false;
  private bool IsWindows(ReadOnlyTargetRules Target)
  {
    return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
  }

	public CarlaTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    // PrivatePCHHeaderFile = "Carla.h";

    if (IsWindows(Target))
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
    if(UsingHoudini)
    {
      PrivateDependencyModuleNames.AddRange(
        new string[]
        {
          "HoudiniEngine",
          "HoudiniEngineEditor",
          "HoudiniEngineRuntime"
        });
    }
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
		AddCarlaServerDependency(Target);
	}

  private bool UseDebugLibs(ReadOnlyTargetRules Target)
  {
    if (IsWindows(Target))
    {
      // In Windows, Unreal uses the Release C++ Runtime (CRT) even in debug
      // mode, so unless we recompile the engine we cannot link the debug
      // libraries.
      return false;
    }
    else
    {
      return false;
    }
  }

  delegate string ADelegate(string s);

  private void AddBoostLibs(string LibPath)
  {
    string [] files = Directory.GetFiles(LibPath, "*boost*.lib");
    foreach (string file in files)
    {
      PublicAdditionalLibraries.Add(file);
    }
  }


	private void AddCarlaServerDependency(ReadOnlyTargetRules Target)
	{
		string LibCarlaInstallPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Carla/CarlaDependencies"));

		ADelegate GetLibName = (string BaseName) => {
			if (IsWindows(Target))
			{
				return BaseName + ".lib";
			}
			else
			{
				return "lib" + BaseName + ".a";
			}
		};

    // Link dependencies.
    if (IsWindows(Target))
    {
      AddBoostLibs(Path.Combine(LibCarlaInstallPath, "lib"));
      PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("rpc")));

      if (UseDebugLibs(Target))
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server_debug")));
      }
      else
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server")));
      }
    }
    else
    {
      PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("rpc")));
      if (UseDebugLibs(Target))
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server_debug")));
      }
      else
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server")));
      }
    }
    // Include path.
    string LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");

    PublicIncludePaths.Add(LibCarlaIncludePath);
    PrivateIncludePaths.Add(LibCarlaIncludePath);

    PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
    // PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
    PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
    PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
	}
}
