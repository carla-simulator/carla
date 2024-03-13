// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using EpicGames.Core;
using System.IO;

public class Carla :
  ModuleRules
{
  [CommandLine("-verbose")]
  bool Verbose = false;

  [CommandLine("-carsim")]
  bool EnableCarSim = false;

  [CommandLine("-chrono")]
  bool EnableChrono = false;

  [CommandLine("-pytorch")]
  bool EnablePytorch = false;

  [CommandLine("-ros2")]
  bool EnableRos2 = false;

  [CommandLine("-osm2odr")]
  bool EnableOSM2ODR = false;



  public Carla(ReadOnlyTargetRules Target) :
    base(Target)
  {
    bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

    PrivatePCHHeaderFile = "Carla.h";
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

    TestOptionalFeature(EnableCarSim, "CarSim support", "WITH_CARSIM");
    TestOptionalFeature(EnableChrono, "Chrono support", "WITH_CHRONO");
    TestOptionalFeature(EnablePytorch, "PyTorch support", "WITH_PYTORCH");
    TestOptionalFeature(EnableRos2, "ROS2 support", "WITH_ROS2");
    TestOptionalFeature(EnableOSM2ODR, "OSM2ODR support", "WITH_OSM2ODR");

    PrivateDependencyModuleNames.AddRange(new string[]
    {
      "AIModule",
      "AssetRegistry",
      "CoreUObject",
      "Engine",
      "Foliage",
      "HTTP",
      "StaticMeshDescription",
      "ImageWriteQueue",
      "Json",
      "JsonUtilities",
      "Landscape",
      "Slate",
      "SlateCore",
      "PhysicsCore",
      "Chaos",
      "ChaosVehicles"
    });

    if (EnableCarSim)
    {
      PrivateDependencyModuleNames.Add("CarSim");
      PrivateIncludePathModuleNames.Add("CarSim");
    }

    PublicDependencyModuleNames.AddRange(new string[]
    {
      "Core",
      "RenderCore",
      "RHI",
      "Renderer",
      "ProceduralMeshComponent",
      "MeshDescription"
    });

    if (EnableCarSim)
      PublicDependencyModuleNames.Add("CarSim");

    if (Target.Type == TargetType.Editor)
      PublicDependencyModuleNames.Add("UnrealEd");
      
    PublicIncludePaths.Add(ModuleDirectory);

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Includes.def")).Split(';'))
      if (Path.Length != 0)
        PublicIncludePaths.Add(Path);

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Libraries.def")).Split(';'))
      if (Path.Length != 0)
        PublicAdditionalLibraries.Add(Path);

    if (EnableOSM2ODR)
    {
      // @TODO
      PublicAdditionalLibraries.Add("");
    }

    if (EnableChrono)
    {
      // @TODO
      var ChronoLibraryNames = new string[]
      {
        "ChronoEngine",
        "ChronoEngine_vehicle",
        "ChronoModels_vehicle",
        "ChronoModels_robot",
      };
    }

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
      PublicDefinitions.AddRange(new string[]
      {
        "NOMINMAX",
        "VC_EXTRALEAN",
        "WIN32_LEAN_AND_MEAN",
      });
    }
  }
}
