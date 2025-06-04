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
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    bEnableExceptions = true;
    bUseRTTI = true;
    
    PublicIncludePaths.Add(ModuleDirectory);

    foreach (var Definition in File.ReadAllText(Path.Combine(PluginDirectory, "Definitions.def")).Split(';'))
    {
      var Trimmed = Definition.Trim();
      if (Trimmed.Length != 0)
        PrivateDefinitions.Add(Trimmed.Trim());
    }

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Includes.def")).Split(';'))
    {
      var Trimmed = Path.Trim();
      if (Trimmed.Length != 0)
        PublicIncludePaths.Add(Trimmed.Trim());
    }

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Libraries.def")).Split(';'))
    {
      var Trimmed = Path.Trim();
      if (Trimmed.Length != 0)
        PublicAdditionalLibraries.Add(Trimmed.Trim());
    }

    foreach (var Option in File.ReadAllText(Path.Combine(PluginDirectory, "Options.def")).Split(';'))
    {
      string Trimmed = Option.Trim();
      switch (Trimmed)
      {
        case "NV_OMNIVERSE":
          EnableNVIDIAOmniverse = true;
          break;
        case "OSM2ODR":
          EnableOSM2ODR = true;
          break;
        default:
          Console.WriteLine($"Unknown option \"{Trimmed}\".");
          break;
      }
    }

    Action<bool, string, string> TestOptionalFeature = (enable, name, definition) =>
    {
      if (enable)
        PrivateDefinitions.Add(name);
      Console.WriteLine(string.Format("{0} is {1}.", name, enable ? "enabled" : "disabled"));
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
      // "StreetMapImporting",
      // "StreetMapRuntime",
      "Chaos",
      "ChaosVehicles",
      "AssetTools",
      "UnrealEd",
      "EditorStyle",
      "AssetRegistry",
      "MeshDescription",
      "MeshUtilitiesCommon",
      "Projects"
    });

    if (EnableNVIDIAOmniverse)
    {
      // @TODO: This should be handled by the (CMake) build system.
      PrivateDefinitions.Add("WITH_OMNIVERSE");

      PrivateDependencyModuleNames.AddRange(new string[]
      {
        "OmniverseUSD",
        "OmniverseRuntime"
      });

      throw new NotImplementedException();
    }

    if (EnableOSM2ODR)
    {
      // @TODO
      throw new NotImplementedException();
    }
  }
}
