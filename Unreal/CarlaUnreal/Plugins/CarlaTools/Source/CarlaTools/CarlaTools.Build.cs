// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using EpicGames.Core;
using System.IO;

public class CarlaTools :
  ModuleRules
{
  [CommandLine("-verbose")]
  bool Verbose = false;

  [CommandLine("-nv-omniverse")]
  bool EnableNVIDIAOmniverse = false;

  [CommandLine("-osm2odr")]
  bool EnableOSM2ODR = false;



  public CarlaTools(ReadOnlyTargetRules Target) :
    base(Target)
  {
    bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    bEnableExceptions = true;
    
    Action<bool, string, string> TestOptionalFeature = (enable, name, definition) =>
    {
      if (enable)
        PrivateDefinitions.Add(name);
      Console.WriteLine(string.Format("{0} is {1}.", name, enable ? "enabled" : "disabled"));
    };
    
    Action<string> AddIncludeDirectories = (str) =>
    {
      if (str.Length == 0)
        return;
      var paths = str.Split(';');
      if (paths.Length == 0)
        return;
      PublicIncludePaths.AddRange(paths);
    };

    TestOptionalFeature(EnableOSM2ODR, "OSM2ODR support", "WITH_OSM2ODR");

    PublicDependencyModuleNames.AddRange(new string[]
    {
      "Core",
      "LevelEditor",
      "ProceduralMeshComponent",
      "MeshDescription",
      "RawMesh",
      "AssetTools",
    });

    PrivateDependencyModuleNames.AddRange(new string[]
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
      "Json",
      "JsonUtilities",
      "Networking",
      "Sockets",
      "HTTP",
      "RHI",
      "RenderCore",
      "MeshMergeUtilities",
      "StreetMapImporting",
      "StreetMapRuntime",
      "Chaos",
      "ChaosVehicles"
    });

    if (EnableNVIDIAOmniverse)
    {
      PrivateDefinitions.Add("WITH_OMNIVERSE");
      PrivateDefinitions.Add("WITH_OMNIVERSE");

      PrivateDependencyModuleNames.AddRange(new string[]
      {
        "OmniverseUSD",
        "OmniverseRuntime"
      });
    }

    if (IsWindows)
    {
      PrivateDefinitions.Add("NOMINMAX");
      PrivateDefinitions.Add("VC_EXTRALEAN");
      PrivateDefinitions.Add("WIN32_LEAN_AND_MEAN");
    }

    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
    PublicDefinitions.Add("BOOST_NO_RTTI");
    PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
    PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
    PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
    PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");

    if (EnableOSM2ODR)
    {
      // @TODO
      PublicAdditionalLibraries.Add("");
    }
    
    PublicIncludePaths.Add(ModuleDirectory);

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Includes.def")).Split(';'))
      if (Path.Length != 0)
        PublicIncludePaths.Add(Path);

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Libraries.def")).Split(';'))
      if (Path.Length != 0)
        PublicAdditionalLibraries.Add(Path);

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

    if (IsWindows)
    {
      PrivateDefinitions.Add("NOMINMAX");
      PrivateDefinitions.Add("VC_EXTRALEAN");
      PrivateDefinitions.Add("WIN32_LEAN_AND_MEAN");
    }
  }
}
