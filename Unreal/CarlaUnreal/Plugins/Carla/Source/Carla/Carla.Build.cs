// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;
using System.Diagnostics;
using EpicGames.Core;

public class Carla :
  ModuleRules
{
  private List<string> ReadCMakeStringOptionsFile(string name)
  {
    var result = new List<string>();
    var path = Path.Combine(PluginDirectory, name + ".def");
    if (File.Exists(path))
    {
      Console.WriteLine("Reading " + name + ".def.");
      foreach (var e in File.ReadAllText(path).Split(';'))
      {
        var entry = e.Trim();
        if (entry.Length != 0)
          result.Add(entry);
      }
    }
    else
    {
      Console.WriteLine("Skipping " + name + ".def, file not found.");
    }
    return result;
  }

  private void AddDynamicLibrary(string library)
  {
    PublicAdditionalLibraries.Add(library);
    RuntimeDependencies.Add(library);
    PublicDelayLoadDLLs.Add(library);
    Console.WriteLine("Dynamic Library Added: " + library);
  }

  public Carla(ReadOnlyTargetRules Target) :
    base(Target)
  {
    foreach (var e in ReadCMakeStringOptionsFile("PublicIncludePaths"))
      PublicIncludePaths.Add(e);
    foreach (var e in ReadCMakeStringOptionsFile("PrivateIncludePaths"))
      PrivateIncludePaths.Add(e);
    foreach (var e in ReadCMakeStringOptionsFile("RuntimeDependencies"))
      RuntimeDependencies.Add(e);
    foreach (var e in ReadCMakeStringOptionsFile("PublicAdditionalLibraries"))
      PublicAdditionalLibraries.Add(e);
    foreach (var e in ReadCMakeStringOptionsFile("PublicDelayLoadDLLs"))
      PublicDelayLoadDLLs.Add(e);

    Console.WriteLine("PluginDirectory: " + PluginDirectory);
    foreach (var option in File.ReadAllText(Path.Combine(PluginDirectory, "Options.def")).Split(';'))
    {
      string optionTrimmed = option.Trim();
      if (optionTrimmed.Equals("ROS2"))
      {
        EnableRos2 = true;
      }
      else if(optionTrimmed.Equals("OSM2ODR"))
      {
        EnableOSM2ODR = true;
      }
    }

    bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

    PrivatePCHHeaderFile = "Carla.h";
    bEnableExceptions = true;
    
    Action<bool, string, string> TestOptionalFeature = (enable, name, definition) =>
    {
      if (enable)
        PrivateDefinitions.Add(definition);
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
        PublicIncludePaths.Add(Path.Trim());

    foreach (var Path in File.ReadAllText(Path.Combine(PluginDirectory, "Libraries.def")).Split(';'))
      if (Path.Length != 0)
        PublicAdditionalLibraries.Add(Path.Trim());

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

    if (EnableRos2)
    {
      TestOptionalFeature(EnableRos2, "Ros2 support", "WITH_ROS2");

      string CarlaPluginSourcePath = Path.GetFullPath(ModuleDirectory);
      string CarlaPluginBinariesLinuxPath = Path.Combine(CarlaPluginSourcePath, "..", "..", "Binaries", "Linux");
      AddDynamicLibrary(Path.Combine(CarlaPluginBinariesLinuxPath, "libcarla-ros2-native.so"));
      RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfoonathan_memory-0.7.3.so"));
      RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastcdr.so"));
      RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastcdr.so.1"));
      RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastcdr.so.1.1.1"));
      RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastrtps.so"));
      RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastrtps.so.2.11"));
      RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastrtps.so.2.11.2"));
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
