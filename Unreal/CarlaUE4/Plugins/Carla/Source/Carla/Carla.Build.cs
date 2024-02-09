// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using System.Linq;
using System.Diagnostics;
using System.Collections.Generic;
using UnrealBuildTool;
using EpicGames.Core;

public class Carla :
  ModuleRules
{
  [CommandLine("-verbose")]
  bool Verbose = true;

  [CommandLine("-carsim")]
  bool EnableCarSim = false;

  [CommandLine("-chrono")]
  bool EnableChrono = false;

  [CommandLine("-pytorch")]
  bool EnablePytorch = false;

  [CommandLine("-ros2")]
  bool EnableRos2 = false;

  [CommandLine("-osm2odr")]
  bool EnableOSM2ODR = true;

  [CommandLine("-carla-install-path")]
  string CarlaInstallPath = null;

  [CommandLine("-carla-dependencies-path")]
  string CarlaDependenciesPath = null;

  public Carla(ReadOnlyTargetRules Target) :
  base(Target)
  {
    bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

    EnableOSM2ODR = IsWindows;

    PrivatePCHHeaderFile = "Carla.h";
    bEnableExceptions = true;

    var DirectoryInfo = new DirectoryInfo(ModuleDirectory);
    for (int i = 0; i != 6; ++i)
      DirectoryInfo = DirectoryInfo.Parent;
    var WorkspacePath = DirectoryInfo.ToString();
    Debug.Assert(WorkspacePath != null && !Directory.Exists(WorkspacePath));

    if (CarlaInstallPath == null)
    {
      Console.WriteLine("\"-carla-install-path\" was not specified, inferring...");
      CarlaInstallPath = Path.Combine(WorkspacePath, "Install");
      if (!Directory.Exists(CarlaInstallPath))
        throw new DirectoryNotFoundException("Could not infer CARLA install directory.");
      Console.WriteLine("Using \"" + CarlaInstallPath + "\" as the CARLA install path.");
    }

    if (CarlaDependenciesPath == null)
    {
      Console.WriteLine("\"-carla-dependencies-path\" was not specified, inferring...");
      CarlaDependenciesPath = Path.Combine(WorkspacePath, "Build", "Dependencies");
      if (!Directory.Exists(CarlaDependenciesPath))
        throw new DirectoryNotFoundException("Could not infer CARLA dependencies directory.");
      Console.WriteLine("Using \"" + CarlaDependenciesPath + "\" as the CARLA depenencies install path.");
    }

    Console.WriteLine("Current module directory: " + ModuleDirectory);

    Action<string, bool> LogBuildFlagStatus = (name, enabled) =>
    {
      Console.WriteLine(
        string.Format(
          "{0} is {1}.",
          name,
          enabled ? "enabled" : "disabled"));
    };

    LogBuildFlagStatus("CarSim support", EnableCarSim);
    LogBuildFlagStatus("Chrono support", EnableChrono);
    LogBuildFlagStatus("PyTorch support", EnablePytorch);
    LogBuildFlagStatus("ROS2 support", EnableRos2);

    if (EnableCarSim)
    {
      PrivateDefinitions.Add("WITH_CARSIM");
      PrivateDefinitions.Add("WITH_CARSIM");
    }

    if (EnableChrono)
    {
      PrivateDefinitions.Add("WITH_CHRONO");
      PrivateDefinitions.Add("WITH_CHRONO");
    }

    if (EnablePytorch)
    {
      PrivateDefinitions.Add("WITH_PYTORCH");
      PrivateDefinitions.Add("WITH_PYTORCH");
    }

    if (EnableRos2)
    {
      PrivateDefinitions.Add("WITH_ROS2");
      PrivateDefinitions.Add("WITH_ROS2");
    }

    if (EnableOSM2ODR)
    {
      PrivateDefinitions.Add("WITH_OSM2ODR");
      PrivateDefinitions.Add("WITH_OSM2ODR");
    }

    // PublicIncludePaths.AddRange(new string[] { });
    // PrivateIncludePaths.AddRange(new string[] { });

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

    // DynamicallyLoadedModuleNames.AddRange(new string[] { });

    var LibraryPrefix = IsWindows ? "" : "lib";
    var LibrarySuffix = IsWindows ? ".lib" : ".a";

    var LibCarlaInstallPath = Path.Combine(CarlaInstallPath, "LibCarla");
    var DependenciesInstallPath = CarlaDependenciesPath;

    Func<string, string> GetLibraryName = name =>
    {
      return LibraryPrefix + name + LibrarySuffix;
    };

    Func<string, string, string[]> FindLibraries = (name, pattern) =>
    {
      var InstallPath = Path.Combine(DependenciesInstallPath, name + "-install");
      var LibPath = Path.Combine(InstallPath, "lib");
      var Candidates = Directory.GetFiles(LibPath, GetLibraryName(pattern));
      if (Candidates.Length == 0)
        throw new FileNotFoundException(string.Format("Could not find any matching libraries for \"{0}\" using pattern \"{1}\"", name, pattern));
      Array.Sort(Candidates);
      return Candidates;
    };

    Func<string, string> GetIncludePath = name => Path.Combine(DependenciesInstallPath, name + "-install", "include");

    // LibCarla
    var LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");
    var LibCarlaLibPath = Path.Combine(LibCarlaInstallPath, "lib");
    var LibCarlaServerPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-server"));
    // var LibCarlaClientPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-client"));

    Debug.Assert(Directory.Exists(LibCarlaServerPath));

    // Boost

    var BoostIncludePath = Path.Combine(DependenciesInstallPath, "boost-install", "include");

    var BoostLibraryPatterns = new string[]
    {
        "boost_date_time*",
        "boost_numpy*",
        "boost_python*",
        "boost_system*",
    };

    if (IsWindows)
    {
      for (int i = 0; i != BoostLibraryPatterns.Length; ++i)
      {
        BoostLibraryPatterns[i] = "lib" + BoostLibraryPatterns[i];
      }
    }

    var SQLiteBuildPath = Path.Combine(DependenciesInstallPath, "sqlite-build");
    var SqliteCandidates = Directory.GetFiles(SQLiteBuildPath, GetLibraryName("*sqlite*"));
    if (SqliteCandidates.Length == 0)
      throw new FileNotFoundException(string.Format("Could not find any matching libraries for SQLite"));
    var SQLiteLibrary = SqliteCandidates[0];
    var AdditionalLibraries = new List<string>
    {
        LibCarlaServerPath,
        SQLiteLibrary,
        FindLibraries("rpclib", "rpc")[0],
        FindLibraries("zlib", IsWindows ? "zlibstatic*" : "z")[0], //TODO: Fix this, note that here we have libz.a and libz.so, need to disambiguate
    };

    foreach (var Pattern in BoostLibraryPatterns)
    {
      var Candidates = FindLibraries("boost", Pattern);
      if (Candidates.Length != 0)
        AdditionalLibraries.Add(Candidates[0]);
    }

    if (EnableOSM2ODR) {
      var XercesCCandidates = FindLibraries("xercesc", "xerces-c*");
      AdditionalLibraries.Add(XercesCCandidates[0]);
      AdditionalLibraries.Add(FindLibraries("proj", "proj")[0]);
      AdditionalLibraries.Add(FindLibraries("sumo", "*osm2odr")[0]);
    }

    if (EnableChrono)
    {
      // Chrono
      var ChronoInstallPath = Path.Combine(DependenciesInstallPath, "chrono-install");
      var ChronoLibPath = Path.Combine(ChronoInstallPath, "lib");
      var ChronoLibraryNames = new string[]
      {
        "ChronoEngine",
        "ChronoEngine_vehicle",
        "ChronoModels_vehicle",
        "ChronoModels_robot",
      };
      var ChronoLibraries =
        from Name in ChronoLibraryNames
        select FindLibraries(ChronoLibPath, GetLibraryName(Name))[0];
      AdditionalLibraries.AddRange(ChronoLibraries);
    }

    PublicIncludePaths.AddRange(new string[]
    {
      ModuleDirectory, // This should probably be removed.
      LibCarlaIncludePath,
      GetIncludePath("boost"),
      GetIncludePath("rpclib"),
      GetIncludePath("zlib"),
      Path.Combine(DependenciesInstallPath, "eigen-source"),
    });

    if (EnableOSM2ODR) {
      PublicIncludePaths.Add(GetIncludePath("xercesc"));
      PublicIncludePaths.Add(GetIncludePath("sumo"));
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

    if (Verbose)
    {
      Console.WriteLine("Additional CARLA libraries:");
      foreach (var e in AdditionalLibraries)
        Console.WriteLine(" - " + e);
    }

    PublicAdditionalLibraries.AddRange(AdditionalLibraries);
  }
}
